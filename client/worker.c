#include "worker.h"

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void run(struct worker *worker_context, struct network_controller manager);
void processingTasks(struct worker *worker_context, struct network_controller *manager);
void initWorkerSignalHandler(sigset_t *empty, sigset_t *block);
void shutdownWorker(struct worker *worker_context, struct network_controller *manager);
struct FileDesc *addNewSocketConnection(struct network_controller *manager, char *domain);
struct FileDesc *findSocketByDomain(char *domain);

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
            currentFD = listViewer->fd.id;

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
            currentFD = listViewer->fd.id;
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
            currentFD = listViewer->fd.id;
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
        socket_pointer = findSocketByDomain(worker_task_pointer->domain);
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