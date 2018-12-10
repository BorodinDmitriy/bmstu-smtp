#include "file_viewer.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

static struct domain_record *Dictionary;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void setUserDirectory(char *dest, char *directory_name);
void processingLetters(struct files_record *files);
char *getDomainFromFile(FILE *file);

//==========================//
//      PUBLIC METHODS      //
//==========================//

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

        printf('fail state');
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
            printf("fail to open directory");
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
            }

            printf("Fail state");
        }

        closedir(pointer->path);
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

struct Mail ReadDataFromFile(int fd)
{
    //  fake
    struct Mail letter;

    letter.from = "IU7.2@yandex.ru";
    letter.to = "IU7.2@yandex.ru";
    letter.message = "Test SMTP";

    return letter;
}

void RevokeLetter(struct Mail letter)
{
    return;
}

void DisposeFileViewer()
{
    //  fake
    return;
}

int GiveControlToFile(struct FileDesc *fd)
{
    return 0;
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
            continue;
        }

        char domain[200];
        strcpy(domain, getDomainFromFile(file));

        fclose(file);

        pointer = pointer->next;
    }

    return;
}

char *getDomainFromFile(FILE *file)
{
    char buffer [LETTER_FRAME_SIZE];
    char result [200];
    char * pointer;
    int len;
    int state = 0;
    while(state == 0) 
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
        strcpy(result, "smtp.");
        strncat(result, pointer + 1, len - 3);
        pointer = result;
        return pointer;
        
    }
    return NULL;
}