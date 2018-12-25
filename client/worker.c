#include "worker.h"

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

void run(struct network_controller manager);
void handler(int sig);

//==========================//
//      PUBLIC METHODS      //
//==========================//
void InitWorker(void *my_info)
{
    struct worker *me = (struct worker *)my_info;

    struct network_controller manager;
    manager.currentState = STATE_START_INIT;

    FD_ZERO(&manager.writers.set);
    manager.writers.count = 0;
    manager.writers.list = NULL;

    FD_ZERO(&manager.readers.set);
    manager.readers.count = 0;
    manager.readers.list = NULL;

    FD_ZERO(&manager.handlers.set);
    manager.handlers.count = 0;
    manager.handlers.list = NULL;

    manager.currentState = STATE_FINISH_INIT;
    run(manager);
    pthread_exit(0);
}

//==========================//
//      PRIVATE METHODS     //
//==========================//
void run(struct network_controller manager)
{
    manager.currentState = STATE_START_WORK;
    
    sigset_t empty_sigs, blocked_sigs;

    sigemptyset(&empty_sigs);
    sigemptyset(&blocked_sigs);

    sigaddset(&blocked_sigs, SIGUSR1);
    sigaddset(&blocked_sigs, SIGINT);
    pthread_sigmask(SIG_BLOCK, &blocked_sigs, &empty_sigs);
    struct sigaction signals;
    signals.sa_handler = handler;
    signals.sa_flags = 0;
    sigemptyset(&signals.sa_mask);
    sigaction(SIGUSR1, &signals, NULL);

    int readyFD = -1;
    int currentFD = 0;
    int processedFD = 0;

    struct FileDescList *listViewer;
    struct FileDesc tempFD;

    struct timespec timer_spec;
    timer_spec.tv_sec = 60;
    timer_spec.tv_nsec = 0;

    fd_set readers_temp;
    fd_set writers_temp;
    fd_set handlers_temp;

    while (true)
    {
        int fd_count = manager.writers.count + manager.readers.count + manager.handlers.count + 1;
        readers_temp = manager.readers.set;
        writers_temp = manager.writers.set;
        handlers_temp = manager.handlers.set;

        readyFD = pselect(fd_count, &readers_temp, &writers_temp, &handlers_temp, &timer_spec, &empty_sigs);
        printf("Wait... pselect : %d\n", GetWorkerPool()->pool[0]->worked);
        if (!GetWorkerPool()->pool[0]->worked) {
            printf("WORKER EXIT\n");
            break;
        }
    
        // if (readyFD == 0)
        // {
        //     //  fake
        //     manager.readers.list = (struct FileDescList *)calloc(1, sizeof(struct FileDescList));
        //     if (manager.readers.list == NULL)
        //     {
        //         printf("Fail to create element on list\r\n");
        //     }

        //     int state = SmtpInitSocket("127.0.0.1", &manager.readers.list->fd);
        //     if (state)
        //     {
        //         printf("Socket not create\r\n");
        //         free(manager.readers.list);
        //     }
        //     state = GiveControlToSocket(&manager.readers.list->fd);
        //     if (state == EWOULDBLOCK) {
        //         FD_SET(manager.readers.list->fd.id, &manager.readers.set);
        //     }
        //     continue;
        // }

        // if (readyFD < 0)
        // {
        //     manager.currentState = STATE_FAIL_WORK;
        //     printf("\nFAIL TO PSELECT\n");
        //     exit(-1);
        // }

        // processedFD = 0;
        // listViewer = manager.readers.list;
        // //  check readers
        // for (int i = 0; i < manager.readers.count; i++)
        // {
        //     //  found ready file description
        //     if (FD_ISSET(currentFD, &readers_temp))
        //     {
        //         //  remove current FD from fd_set
        //         FD_CLR(currentFD, &manager.readers.set);

        //         tempFD = listViewer->fd;
        //         int fdState = 0;
        //         if (tempFD.type == SOCKET_FD)
        //         {
        //             fdState = GiveControlToSocket(&tempFD);
        //         }
        //         else
        //         {
        //             // fdState = GiveControlToFile(&tempFD);
        //         }

        //         fdState++;

        //         processedFD++;
        //         if (processedFD == readyFD)
        //         {
        //             break;
        //         }
        //     }
        // }

        //     if ()
        //     {
        //         //  client ready to read from file
        //         struct Mail letter = ReadDataFromFile(0);
        //         SendMail(readyFD, letter);
        //         RevokeLetter(letter);
        //         continue;
        //     }

        //     if (FD_ISSET(readyFD, &manager.writers.set))
        //     {
        //         //  client ready to send data
        //         continue;
        //     }

        //     if (FD_ISSET(readyFD, &manager.handlers.set))
        //     {
        //         //  client receive interrupt
        //         break;
        //     }
        // }
    }
    printf("Exit\n");
    return;
}

void handler(int signum) {
    if (signum == SIGUSR1)
        printf("Main want to i kill self\n");
}