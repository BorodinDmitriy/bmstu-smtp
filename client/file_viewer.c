#include "file_viewer.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

static struct domain_record *Dictionary;
static sem_t lock;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void setUserDirectory(char *dest, char *directory_name);
void processingLetters(struct files_record *files);
int setDomainFromFile(char *domain, FILE *file);
int setPathInTempDirectory(char *dest, char *path);
int moveLetter(char *from, char *to);
struct worker_task *createTaskForLetter(char *domain, char *path);
void destroyTask(struct worker_task *task);

//  Dictionary methods
void freeDicrionary();
int addRecordToDictionary(char *domain);

//==========================//
//      PUBLIC METHODS      //
//==========================//

int InitFileViewer()
{
    Dictionary = NULL;
    int err;
    err = sem_init(&lock, 0, 1);
    if (err != 0)
    {
        printf("FileViewer: initialization fail. Exit...");
        return -1;
    }
}

void DestroyFileViewer()
{
    int err;
    freeDicrionary();
    err = sem_destroy(&lock);
    if (err != 0)
    {
        printf("FileViewer: fail to destroy semaphore.");
    }
    return;
}

void SearchNewFiles()
{
    DIR *directory;
    struct dirent *dir;
    directory = opendir(CLIENT_MAIL_DIR_PATH);
    if (!directory)
    {
        printf("users not found");
        return;
    }

    //  save users maildir
    struct files_record *queue = NULL;
    struct files_record *pointer = queue;
    while ((dir = readdir(directory)) != NULL)
    {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
        {
            continue;
        }

        if (queue == NULL)
        {
            queue = (struct files_record *)malloc(sizeof(struct files_record));
            if (queue == NULL)
            {
                printf("fail to init queue");
                continue;
            }

            setUserDirectory(queue->path, dir->d_name);
            queue->next = NULL;
            pointer = queue;
            continue;
        }

        if (pointer->next == NULL)
        {
            struct files_record *queue_item;
            queue_item = (struct files_record *)malloc(sizeof(struct files_record));
            if (queue_item == NULL)
            {
                printf("Fail to create record of directory with name %s", dir->d_name);
                continue;
            }
            setUserDirectory(queue_item->path, dir->d_name);
            queue_item->next = NULL;
            pointer->next = queue_item;
            pointer = pointer->next;
            continue;
        }

        printf("fail state");
    }

    //  Close maildir for program;
    closedir(directory);

    //  Check new's directory of find users
    pointer = queue;
    struct files_record *file_queue = NULL;
    struct files_record *file_pointer = NULL;
    while (pointer != NULL)
    {
        directory = opendir(pointer->path);
        if (!directory)
        {
            Error("fail to open directory");
            printf("fail to open directory\n");
            closedir(directory);
            pointer = pointer->next;
            continue;
        }

        //  Read files in directory;
        while ((dir = readdir(directory)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }

            if (file_queue == NULL)
            {
                file_queue = (struct files_record *)malloc(sizeof(struct files_record));
                if (file_queue == NULL)
                {
                    printf("fail to init file queue");
                    continue;
                }

                strcpy(file_queue->path, pointer->path);
                strcat(file_queue->path, dir->d_name);
                file_queue->next = NULL;
                file_pointer = file_queue;
                continue;
            }

            if (file_pointer->next == NULL)
            {
                struct files_record *queue_item = (struct files_record *)malloc(sizeof(struct files_record));
                if (queue_item == NULL)
                {
                    printf("Fail to create record of files");
                    continue;
                }

                strcpy(queue_item->path, pointer->path);
                strcat(queue_item->path, dir->d_name);
                queue_item->next = NULL;
                file_pointer->next = queue_item;
                file_pointer = queue_item;
                continue;
            }

            printf("Fail state");
        }

        closedir(directory);
        pointer = pointer->next;
    }

    //  free directory queue;
    pointer = queue;
    while (pointer != NULL)
    {
        pointer = pointer->next;
        free(queue);
    }

    processingLetters(file_queue);
    return;
}

int FindDomainInDictionary(char *domain)
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;
    while (pointer != NULL)
    {
        if (strcmp(pointer->domain, domain) == 0)
        {
            return pointer->workerId;
        }
    }

    sem_post(&lock);
    return -1;
}

void RemoveDomainRecordFromDictionary(int workerId, char *domain)
{
    
}

//==========================//
//      PRIVATE METHODS     //
//==========================//

//  set path of new letters of user, witch owner "directory_name"
void setUserDirectory(char *dest, char *directory_name)
{
    strcpy(dest, CLIENT_MAIL_DIR_PATH);
    strcat(dest, directory_name);
    strcat(dest, "/Maildir/new/");
    return;
}

//  processing letters: find domain in letter and send on work
void processingLetters(struct files_record *files)
{
    struct files_record *pointer = files;
    FILE *file;
    while (pointer != NULL)
    {
        file = fopen(pointer->path, "r");
        if (!file)
        {
            printf("Fail to open file : %s", pointer->path);
            pointer = pointer->next;
            continue;
        }

        char domain[200];
        int state = setDomainFromFile(domain, file);
        if (state)
        {
            printf("Fail to find domain in letter %s", files->path);
            pointer = pointer->next;
            continue;
        }

        fclose(file);

        //  create task for worker with index @workerId@ in pool
        struct worker_task *task = createTaskForLetter(domain, pointer->path);

        int workerId = FindDomainInDictionary(domain);
        int tryes = 0;
        while (workerId < 0 && tryes < 3)
        {
            //  worker not found
            workerId = addRecordToDictionary(domain);
            tryes++;
        }

        if (workerId < 0) 
        {
            destroyTask(task);
            pointer = pointer->next;
            continue;
        }

        moveLetter(pointer->path, task->path);
        state = DelegateTaskToWorker(workerId, task);
        if (state) 
        {
            moveLetter(task->path, pointer->path);
            destroyTask(task);
        }
        pointer = pointer->next;
    }

    return;
}

//  get domain from letter by field "To:"
int setDomainFromFile(char *domain, FILE *file)
{
    char buffer[LETTER_FRAME_SIZE];
    char *pointer;
    int len;
    int state = 0;
    while (state == 0)
    {
        if (fgets(buffer, LETTER_FRAME_SIZE, file) == NULL)
        {
            break;
        }

        len = strlen(buffer);
        if (buffer[len - 2] != '\r' && buffer[len - 1] != '\n')
        {
            continue;
        }

        if (buffer[0] != 'T' && buffer[1] != 'o' && buffer[2] != ':')
        {
            continue;
        }

        pointer = strstr(buffer, "@");
        if (!pointer)
        {
            continue;
        }

        len = strlen(pointer);
        strncpy(domain, pointer + 1, len - 3);
        return 0;
    }
    return -1;
}

struct worker_task *createTaskForLetter(char *domain, char *path)
{
    struct worker_task *task = (struct worker_task *)malloc(sizeof(struct worker_task));
    if (!task)
    {
        printf("Fail to create task by letter");
        return NULL;
    }

    task->state = NEW_TASK;

    int len = strlen(path) + 1;
    task->path = (char *)calloc(len, sizeof(char));
    if (!task->path) 
    {
        printf("\nFail to create path in task");
        free(task);
        return NULL;
    }
    
    int state = setPathInTempDirectory(task->path, path);
    if (state) 
    {
        printf("Fail to receive tmp place of file");
        free(task);
        return NULL;
    }

    len = strlen(domain) + 1;
    task->domain = (char *)calloc(len, sizeof(char));
    if (!task->domain)
    {
        printf("\nFail to create domain field in task");
        free(task->path);
        free(task);
        return NULL;
    }
    
    memset(task->domain, '\0', len);
    strncpy(task->domain, domain, len);
    task->next = NULL;
    return task;
}

void destroyTask(struct worker_task *task)
{
    free(task->domain);
    free(task->path);
    free(task);
    return;
}

int setPathInTempDirectory(char *dest, char *path)
{
    char *pattern = "/new/";
    char *pos = strstr(path, pattern);
    int count = pos - path;
    strncpy(dest, path, count);
    strcat(dest, "/tmp/");
    strcat(dest, path + count + 5);
    return 0;
}

int moveLetter(char *from, char *to)
{
    int state = 0;
    state = rename(from, to);
    if (state < 0) 
    {
        printf("Fail to move file from %s to %s", from, to);   
    }
    return state;
}

//  Dictionary section
int addRecordToDictionary(char *domain)
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;

    while (Dictionary && pointer->next != NULL)
    {
        pointer = pointer->next;
    }

    struct domain_record *new_record = (struct domain_record *)malloc(sizeof(struct domain_record));
    if (new_record == NULL)
    {
        printf("FileView: fail to create new record for dictionary");
        sem_post(&lock);
        return -1;
    }

    int len = strlen(domain) + 1;
    new_record->domain = (char *)calloc(len, sizeof(char));
    if (new_record->domain == NULL)
    {
        printf("FileView: fail to allocate memory for domain\n");
        free(new_record);
        sem_post(&lock);
        return -1;
    }

    memset(new_record->domain, '\0', len);
    strcpy(new_record->domain, domain);
    new_record->workerId = MostFreeWorker();
    new_record->next = NULL;

    if (Dictionary == NULL)
    {    
        Dictionary = new_record;
    } 
    else 
    {
        pointer->next = new_record;
    }

    sem_post(&lock);
    return 0;
}

void freeDicrionary()
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;
    while (pointer != NULL)
    {
        pointer = pointer->next;
        free(Dictionary);
        Dictionary = pointer;
    }
    sem_post(&lock);

    return;
}