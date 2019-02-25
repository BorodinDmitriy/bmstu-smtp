#include "worker.h"

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void run(struct worker *worker_context, struct network_controller manager);
void processingTasks(struct worker *worker_context, struct network_controller *manager);
void initWorkerSignalHandler(sigset_t *empty, sigset_t *block);
void closingConnections(struct worker *worker_context, struct network_controller *manager);
void shutdownWorker(struct worker *worker_context, struct network_controller *manager);
struct FileDesc *addNewSocketConnection(struct network_controller *manager, char *domain);
struct FileDesc *findSocketByDomain(struct network_controller *manager, char *domain);

void handler(int sig);

//==========================//
//      PUBLIC METHODS      //
//==========================//
void InitWorker(void *my_info)
{
    struct worker *me = (struct worker *)my_info;
    printf("\tWorker %d start initing\n", me->workerId);

    struct network_controller manager;

    FD_ZERO(&manager.writers.set);
    manager.writers.count = 0;

    FD_ZERO(&manager.readers.set);
    manager.readers.count = 0;

    FD_ZERO(&manager.handlers.set);
    manager.handlers.count = 0;

    manager.socket_list = NULL;
    printf("\tWorker %d finish initinig\n", me->workerId);
    run(my_info, manager);
    printf("\tWorker %d: exit with 0\n", me->workerId);
    pthread_exit(NULL);

    return;
}

//==========================//
//      PRIVATE METHODS     //
//==========================//
void run(struct worker *worker_context, struct network_controller manager)
{
    sigset_t empty_sigs, blocked_sigs;

    initWorkerSignalHandler(&empty_sigs, &blocked_sigs);

    int readyFD = -1;
    int currentFD = 0;
    int status = 0;
    bool flag_of_belonging;
    bool flag_of_detect;
    int closing = 0;

    struct FileDescList *listViewer;

    struct timespec timer_spec;
    timer_spec.tv_sec = 60;
    timer_spec.tv_nsec = 0;

    fd_set readers_temp;
    fd_set writers_temp;
    fd_set handlers_temp;

    while (worker_context->worked || manager.socket_list != NULL)
    {
        int fd_count = 1;
        listViewer = manager.socket_list;
        while (listViewer)
        {
            fd_count = MAX(fd_count, listViewer->fd->id) + 1;
            printf("\tWorker %d: domain %s state: %d\n",worker_context->workerId, listViewer->fd->domain, listViewer->fd->current_state);
            listViewer = listViewer->next;
        }
        readers_temp = manager.readers.set;
        writers_temp = manager.writers.set;
        handlers_temp = manager.handlers.set;

        printf("\n");
        readyFD = pselect(fd_count, &readers_temp, &writers_temp, &handlers_temp, &timer_spec, &empty_sigs);
        printf("\tWorker %d: unsleep\n", worker_context->workerId);

        if (!(worker_context->worked) && closing == 0)
        {
            printf("\tWorker %d: closing connections for shutdown\n", worker_context->workerId);
            closingConnections(worker_context, &manager);
            closing++;
            timer_spec.tv_sec = 5;
        }

        if (closing != 0)
        {
            readers_temp = manager.readers.set;
            writers_temp = manager.writers.set;
            readyFD = 1;
        }


        if (worker_context->count_task == 0)
        {
            printf("\tWorker %d: I don't have task\n", worker_context->workerId);
            continue;
        }

        processingTasks(worker_context, &manager);

        if (readyFD <= 0)
        {
            printf("\tWorker %d: run: not ready sockets\n", worker_context->workerId);
            continue;
        }

        listViewer = manager.socket_list;
        //  check all sockets readers, writers and handlers
        while (listViewer != NULL)
        {
            currentFD = listViewer->fd->id;
            flag_of_detect = false;

            flag_of_belonging = FD_ISSET(currentFD, &readers_temp);

            //  ready reader
            if (flag_of_belonging)
            {
                FD_CLR(currentFD, &manager.readers.set);
                status = 0;
                while (status == 0)
                {
                    status = SMTP_Control(listViewer->fd);

                    //  we need to handle error
                    if (listViewer->fd->current_state == SMTP_ERROR)
                    {
                        status = 0;
                    }

                    if (listViewer->fd->prev_state == DISPOSING_SOCKET)
                    {
                        break;
                    }
                }
                printf("\tWorker %d: status(%d)\n",worker_context->workerId, status);
                if (status != 1 && status != 0)
                {
                    // unsolve exception or all ok, still need to remove record from dictionary and delete socket
                    RemoveDomainRecordFromDictionary(listViewer->fd->domain);
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                }
                else if (status == 0)
                {
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                }
                else if (status == 1 && !worker_context->worked && listViewer->fd->task_pool == NULL)
                {
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                    printf("\tremove\n");
                }
                else 
                    flag_of_detect = true;
            }

            flag_of_belonging = FD_ISSET(currentFD, &writers_temp);

            //  ready writer
            if (!flag_of_detect && flag_of_belonging)
            {
                FD_CLR(currentFD, &manager.writers.set);
                status = 0;
                while (status == 0)
                {
                    status = SMTP_Control(listViewer->fd);

                    //  we need to handle error
                    if (listViewer->fd->current_state == SMTP_ERROR)
                    {
                        status = 0;
                    }

                    if (listViewer->fd->prev_state == DISPOSING_SOCKET)
                    {
                        break;
                    }
                }

                if (status != 1 && status != 0)
                {
                    // unsolve exception or all ok, still need to remove record from dictionary and delete socket
                    RemoveDomainRecordFromDictionary(listViewer->fd->domain);
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                } 
                else if (status == 0)
                {
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                }
                else if (status == 1 && !worker_context->worked && listViewer->fd->task_pool == NULL)
                {
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                }
                else 
                    flag_of_detect = true;
            }

            if (flag_of_detect && status == 1)
            {
                //  socket will blocked, resolve to which set the socket belongs

                //  readers
                if (listViewer->fd->current_state == RECEIVE_SMTP_GREETING ||
                    listViewer->fd->current_state == RECEIVE_EHLO_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_MAIL_FROM_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_RCPT_TO_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_DATA_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_LETTER_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_QUIT_RESPONSE ||
                    listViewer->fd->current_state == RECEIVE_RSET_RESPONSE)
                {
                    FD_SET(currentFD, &manager.readers.set);
                }
                else if (listViewer->fd->current_state == SEND_EHLO ||
                         listViewer->fd->current_state == SEND_MAIL_FROM ||
                         listViewer->fd->current_state == SEND_RCPT_TO ||
                         listViewer->fd->current_state == SEND_DATA ||
                         listViewer->fd->current_state == SEND_LETTER ||
                         listViewer->fd->current_state == SEND_QUIT ||
                         listViewer->fd->current_state == SEND_RSET)
                {
                    FD_SET(currentFD, &manager.writers.set);
                }
                else
                {
                    char message[200];
                    memset(message, '\0', 200);
                    sprintf(message, "Worker: run: Fail SMTP Graph STATE: current(%d), prev(%d)", listViewer->fd->current_state, listViewer->fd->prev_state);
                    Error(message);

                    //  Something went wrong and I don't know how to resolve current graph state;
                    RemoveDomainRecordFromDictionary(listViewer->fd->domain);
                    removeSocketConnectionFromPool(&manager, listViewer->fd);
                }
            }

            listViewer = listViewer->next;
        }
    }
    printf("\tWorker %d: cancel pselect\n", worker_context->workerId);
    shutdownWorker(worker_context, &manager);
    return;
}

void processingTasks(struct worker *worker_context, struct network_controller *manager)
{
    struct FileDesc *socket_pointer;
    struct worker_task *socket_task_pointer;
    printf("\tWorker %d: start processing tasks\n", worker_context->workerId);
    sem_wait(&worker_context->lock);
    while (worker_context->tasks)
    {
        socket_pointer = findSocketByDomain(manager, worker_context->tasks->domain);
        //  if socket with current domain is exist
        if (socket_pointer)
        {
            socket_task_pointer = socket_pointer->task_pool;
            if (socket_task_pointer == NULL)
            {
                socket_pointer->task_pool = worker_context->tasks;
                worker_context->tasks = worker_context->tasks->next;
                socket_pointer->task_pool->next = NULL;
                continue;
            }

            while (socket_task_pointer->next != NULL)
            {
                socket_task_pointer = socket_task_pointer->next;
            }

            socket_task_pointer->next = worker_context->tasks;
            worker_context->tasks = worker_context->tasks->next;
            socket_task_pointer = socket_task_pointer->next;
            socket_task_pointer->next = NULL;
            continue;
        }

        //  socket with current domain isn't exist
        socket_pointer = addNewSocketConnection(manager, worker_context->tasks->domain);
        if (!socket_pointer || socket_pointer->id < 0)
        {
            char message[100];
            memset(message, '\0', 100);
            sprintf(message, "Worker %d has problem on create new socket connection. Current record skipped", worker_context->workerId);
            Error(message);
            //  Revert TASK

            int len = strlen(worker_context->tasks->path);
            char *dest = calloc(len + 1, sizeof(char));
            SetPathInNewDirectory(dest, worker_context->tasks->path);
            MoveLetter(worker_context->tasks->path, dest);
            free(dest);

            struct worker_task *task_pointer = worker_context->tasks->next;
            DestroyTask(worker_context->tasks);

            worker_context->tasks = task_pointer;
            continue;
        }

        FD_SET(socket_pointer->id, &manager->readers.set);
        // FD_SET(socket_pointer->id, &manager->writers.set);

        socket_pointer->task_pool = worker_context->tasks;
        worker_context->tasks = worker_context->tasks->next;
        socket_pointer->task_pool->next = NULL;
        continue;
    }

    sem_post(&worker_context->lock);
    printf("\tWorker %d: finish processing tasks\n", worker_context->workerId);
    return;
}

void shutdownWorker(struct worker *worker_context, struct network_controller *manager)
{
    free(manager->socket_list);
    return;
}

void closingConnections(struct worker *worker_context, struct network_controller *manager)
{
    struct FileDescList *listViewer = manager->socket_list;
    struct FileDescList *connector = NULL;
    struct worker_task *task_pointer = NULL;
    struct worker_task *next_task_pointer = NULL;
    int len;
    char *filepath;
    int state;
    printf("\tWorker %d: start remove other tasks\n", worker_context->workerId);
    while (listViewer)
    {
        printf("\tWorker %d: remove domain %s from dictionary\n", worker_context->workerId, listViewer->fd->domain);
        RemoveDomainRecordFromDictionary(listViewer->fd->domain);
        printf("\tWorker %d: success removing domain %s from dictionary\n", worker_context->workerId, listViewer->fd->domain);
        task_pointer = listViewer->fd->task_pool;
        if (task_pointer != NULL)
        {
            printf("\tWorker %d: task pool is not empty for domain %s\n", worker_context->workerId, listViewer->fd->domain);
            if (listViewer->fd->current_state >= 2)
            {
                task_pointer = task_pointer->next;
                listViewer->fd->task_pool->next = NULL;
            }
            else
            {
                listViewer->fd->task_pool = NULL;
            }
            
            while (task_pointer)
            {
                printf("\tWorker %d: remove next task in pool for domain %s\n", worker_context->workerId, listViewer->fd->domain);
                next_task_pointer = task_pointer->next;
                len = strlen(task_pointer->path);
                printf("\tWorker %d: file %s moving\n", worker_context->workerId, task_pointer->path);
                filepath = (char *)calloc(len + 1, sizeof(char));
                if (!filepath)
                {
                    char message[100];
                    memset(message, '\0', 100);
                    sprintf(message, "Worker %d: closingConnections:  has problem to allocate memory for filepath (%s)", worker_context->workerId, task_pointer->path);
                    Error(message);
                }
                else
                {
                    memset(filepath, '\0', len + 1);
                    state = SetPathInNewDirectory(filepath, task_pointer->path);
                    if (state != 0)
                    {
                        char message[100];
                        memset(message, '\0', 100);
                        sprintf(message, "Worker %d: closingConnections:  has problem to set new filepath for file(%s)", worker_context->workerId, task_pointer->path);
                        Error(message);
                    }
                    MoveLetter(task_pointer->path, filepath);
                }
                free(filepath);
                DestroyTask(task_pointer);
                task_pointer = next_task_pointer;
            }
            
        }

        connector = listViewer;
        listViewer = listViewer->next;
    }
    return;
}

//======================//
//  SOCKET POOL TOOLS   //
//======================//

struct FileDesc *findSocketByDomain(struct network_controller *manager, char *domain)
{
    struct FileDescList *pointer = manager->socket_list;
    struct FileDesc *result = NULL;
    int state;
    while (pointer != NULL)
    {
        state = strcmp(pointer->fd->domain, domain);
        if (state == 0)
        {
            result = pointer->fd;
            break;
        }

        pointer = pointer->next;
    }

    return result;
}

struct FileDesc *addNewSocketConnection(struct network_controller *manager, char *domain)
{
    int state;
    struct FileDesc *new_socket_connection = (struct FileDesc *)malloc(sizeof(struct FileDesc));
    if (!new_socket_connection)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker has problem on allocate memory by new socket connection.");
        Error(message);
        return NULL;
    }

    //  copy domain
    int len;
    len = strlen(domain) + 1;
    new_socket_connection->domain = (char *)calloc(len, sizeof(char));
    if (!new_socket_connection->domain)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker has problem on allocate memory by socket domain.");
        Error(message);
        free(new_socket_connection);
    }
    memset(new_socket_connection->domain, '\0', len);
    strncpy(new_socket_connection->domain, domain, len - 1);

    struct FileDescList *pointer = manager->socket_list;
    struct FileDescList *new_record = (struct FileDescList *)malloc(sizeof(struct FileDescList));
    if (!new_record)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker has problem on allocate memory by new record in file desc list.");
        Error(message);
        free(new_socket_connection->domain);
        free(new_socket_connection);
    }

    new_record->fd = new_socket_connection;
    new_record->next = NULL;

    if (pointer)
    {
        while (pointer->next != NULL)
        {
            pointer = pointer->next;
        }
        pointer->next = new_record;
    }
    else
    {
        manager->socket_list = new_record;
    }

    new_socket_connection->prev_state = NULL_POINTER;
    new_socket_connection->current_state = PREPARE_SOCKET_CONNECTION;
    state = SMTP_Control(new_socket_connection);
    if (state != 0 && state != 1)
    {
        char message[100];
        memset(message, '\0', 100);
        sprintf(message, "Worker has problem on preparing socket for new connection by domain(%s)", new_socket_connection->domain);
        Error(message);

        //  free all allocated fields
        free(new_record);
        if (pointer)
        {
            pointer->next = NULL;
        }
        else
        {
            manager->socket_list = NULL;
        }

        free(new_socket_connection->mx_record);
        free(new_socket_connection->domain);
        free(new_socket_connection);
    }

    return new_socket_connection;
}

void removeSocketConnectionFromPool(struct network_controller *manager, struct FileDesc *socket_connection)
{
    struct FileDescList *pool_pointer = manager->socket_list;
    struct FileDescList *pool_pointer_prev = NULL;
    while (pool_pointer)
    {
        if (pool_pointer->fd->id == socket_connection->id)
        {
            //  Removing
            if (pool_pointer_prev != NULL)
            {
                pool_pointer_prev->next = pool_pointer->next;
            }
            else if (pool_pointer->next)
            {
                manager->socket_list = manager->socket_list->next;
            }
            else
            {
                free(socket_connection->domain);
                free(socket_connection);
                free(manager->socket_list);
                manager->socket_list = NULL;
                break;
            }

            free(socket_connection->domain);
            free(socket_connection);
            free(pool_pointer);
            break;
        }

        pool_pointer_prev = pool_pointer;
        pool_pointer = pool_pointer->next;
    }
}

void removeAllSocketConnectionByDomain(struct network_controller *manager)
{
    struct FileDescList *pointer = manager->socket_list;
    while (manager->socket_list)
    {
        pointer = manager->socket_list->next;
        free(manager->socket_list->fd->domain);
        free(manager->socket_list->fd);
        free(manager->socket_list);
        manager->socket_list = pointer;
    }

    return;
}

//======================//
//    SIGNAL FIELD      //
//======================//

void initWorkerSignalHandler(sigset_t *empty, sigset_t *block)
{
    sigemptyset(empty);
    sigemptyset(block);

    sigaddset(block, SIGUSR1);
    sigaddset(block, SIGINT);
    pthread_sigmask(SIG_BLOCK, block, empty);
    // struct sigaction signals;
    // signals.sa_handler = handler;
    // signals.sa_flags = 0;
    // sigemptyset(&signals.sa_mask);
    // sigaction(SIGUSR1, &signals, NULL);
    return;
}

// void handler(int signum)
// {
//     if (signum == SIGUSR1)
//         printf("\tWorker unsleep by main thread\n");

//     return;
// }
