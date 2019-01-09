#include "worker.h"

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void run(struct worker *worker_context, struct network_controller manager);
void processingTasks(struct worker *worker_context, struct network_controller *manager);
void initWorkerSignalHandler(sigset_t *empty, sigset_t *block);
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
    pthread_exit(0);
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
    int processedFD = 0;

    struct FileDescList *listViewer;

    struct timespec timer_spec;
    timer_spec.tv_sec = 60;
    timer_spec.tv_nsec = 0;

    fd_set readers_temp;
    fd_set writers_temp;
    fd_set handlers_temp;

    while (worker_context->worked)
    {
        int fd_count = manager.writers.count + manager.readers.count + manager.handlers.count + 1;
        readers_temp = manager.readers.set;
        writers_temp = manager.writers.set;
        handlers_temp = manager.handlers.set;

        readyFD = pselect(fd_count, &readers_temp, &writers_temp, &handlers_temp, &timer_spec, &empty_sigs);
        printf("\tWorker %d start working\n", worker_context->workerId);

        if (!(worker_context->worked))
        {
            printf("\tWorker %d start closing\n");
            break;
        }

        if (worker_context->count_task == 0)
        {
            printf("\tWorker %d: I don't have task\n");
            continue;
        }

        processingTasks(worker_context, &manager);

        if (readyFD < 0)
        {
            printf("\nWorker %d has problem on pselect\n", worker_context->workerId);
            char message[100];
            memset(message, '\0', 100);
            sprintf(message, "Worker %d has problem on pselect. Current state: STATE_FAIL_WORK. It's over", worker_context->workerId);
            Error(message);
            break;
        }

        processedFD = 0;
        listViewer = manager.socket_list;
        //  check readers
        for (int i = 0; i < manager.readers.count; i++)
        {
            currentFD = listViewer->fd->id;

            //  found ready file description
            if (FD_ISSET(currentFD, &readers_temp))
            {
                //  remove current FD from fd_set
                FD_CLR(currentFD, &manager.readers.set);

                //  Processing Reader socket

                processedFD++;
                if (processedFD == readyFD)
                {
                    break;
                }
            }

            listViewer = listViewer->next;
        }

        listViewer = manager.socket_list;
        for (int i = 0; i < manager.writers.count; i++)
        {
            currentFD = listViewer->fd->id;
            if (FD_ISSET(currentFD, &writers_temp))
            {
                FD_CLR(currentFD, &manager.writers.set);

                //  Processing Writers sockets

                processedFD++;
                if (processedFD == readyFD)
                {
                    break;
                }
            }

            listViewer = listViewer->next;
        }

        listViewer = manager.socket_list;
        for (int i = 0; i < manager.handlers.count; i++)
        {
            currentFD = listViewer->fd->id;
            if (FD_ISSET(currentFD, &handlers_temp))
            {
                FD_CLR(currentFD, &manager.handlers.set);

                printf("\tWorker %d receive handler event\n", worker_context->workerId);

                processedFD++;
                if (processedFD == readyFD)
                {
                    break;
                }
            }

            listViewer = listViewer->next;
        }
    }
    printf("\tWorker %d cancel pselect\n", worker_context->workerId);
    shutdown(worker_context, &manager);
    return;
}

void processingTasks(struct worker *worker_context, struct network_controller *manager)
{
    struct worker_task *worker_task_pointer = worker_context->tasks;
    struct FileDesc *socket_pointer;
    struct worker_task *socket_task_pointer;
    sem_wait(&worker_context->lock);
    while (worker_task_pointer)
    {
        socket_pointer = findSocketByDomain(manager, worker_task_pointer->domain);
        //  if socket with current domain is exist
        if (socket_pointer)
        {
            socket_task_pointer = socket_pointer->task_pool;
            if (socket_task_pointer == NULL)
            {
                socket_pointer->task_pool = worker_task_pointer;
                worker_task_pointer = worker_task_pointer->next;
                socket_pointer->task_pool->next = NULL;
                continue;
            }

            while (socket_task_pointer->next != NULL)
            {
                socket_task_pointer = socket_task_pointer->next;
            }

            socket_task_pointer->next = worker_task_pointer;
            worker_task_pointer = worker_task_pointer->next;
            socket_task_pointer = socket_task_pointer->next;
            socket_task_pointer->next = NULL;
            continue;
        }

        //  socket with current domain isn't exist
        socket_pointer = addNewSocketConnection(manager, worker_task_pointer->domain);
        if (!socket_pointer)
        {
            char message[100];
            memset(message, '\0', 100);
            sprintf(message, "Worker %d has problem on create new socket connection. Current record skipped", worker_context->workerId);
            Error(message);
            worker_task_pointer = worker_task_pointer->next;
            continue;
        }

        socket_pointer->task_pool = worker_task_pointer;
        worker_task_pointer = worker_task_pointer->next;
        socket_pointer->task_pool->next = NULL;
        continue;
    }

    sem_post(&worker_context->lock);
    return;
}

void shutdownWorker(struct worker *worker_context, struct network_controller *manager)
{
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
    if (state != 0)
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
    struct FileDescList *pool_pointer_prev;
    while (pool_pointer)
    {
        if (pool_pointer->fd->id == socket_connection->id)
        {
            pool_pointer_prev->next = pool_pointer->next;

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
    struct sigaction signals;
    signals.sa_handler = handler;
    signals.sa_flags = 0;
    sigemptyset(&signals.sa_mask);
    sigaction(SIGUSR1, &signals, NULL);
    return;
}

void handler(int signum)
{
    if (signum == SIGUSR1)
        printf("\tWorker unsleep by main thread\n");

    return;
}