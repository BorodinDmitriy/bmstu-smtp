#include "controller.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//
static struct Controller Manager;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitController()
{
    Manager.currentState = STATE_START_INIT;
    Manager.worked = false;

    FD_ZERO(&Manager.writers.set);
    Manager.writers.count = 0;
    Manager.writers.list = NULL;

    FD_ZERO(&Manager.readers.set);
    Manager.readers.count = 0;
    Manager.readers.list = NULL;

    FD_ZERO(&Manager.handlers.set);
    Manager.handlers.count = 0;
    Manager.handlers.list = NULL;

    Manager.currentState = STATE_FINISH_INIT;
    return;
}

void Run()
{
    Manager.currentState = STATE_START_WORK;
    Manager.worked = true;

    int readyFD = -1;
    int currentFD = 0;
    int processedFD = 0;

    struct FileDescList *listViewer;
    struct FileDesc tempFD;

    struct timespec timer_spec;
    timer_spec.tv_sec = 10;
    timer_spec.tv_nsec = 0;

    fd_set readers_temp;
    fd_set writers_temp;
    fd_set handlers_temp;

    while (Manager.worked)
    {
        int fd_count = Manager.writers.count + Manager.readers.count + Manager.handlers.count + 1;
        readers_temp = Manager.readers.set;
        writers_temp = Manager.writers.set;
        handlers_temp = Manager.handlers.set;

        readyFD = pselect(fd_count, &readers_temp, &writers_temp, &handlers_temp, &timer_spec, NULL);

        if (readyFD == 0)
        {
            //  fake 
            Manager.readers.list = (struct FileDescList *)calloc(1, sizeof(struct FileDescList));
            if (Manager.readers.list == NULL)
            {
                printf("Fail to create element on list\r\n");
            }

            int state = SmtpInitSocket("127.0.0.1", &Manager.readers.list->fd);
            if (state) 
            {
                printf("Socket not create\r\n");
                free(Manager.readers.list);
            }
            state = GiveControlToSocket(&Manager.readers.list->fd);
            if (state == EWOULDBLOCK) {
                FD_SET(Manager.readers.list->fd.id, &Manager.readers.set);
            }
            continue;
        }

        if (readyFD < 0)
        {
            Manager.currentState = STATE_FAIL_WORK;
            printf("\nFAIL TO PSELECT\n");
            exit(-1);
        }

        processedFD = 0;
        listViewer = Manager.readers.list;
        //  check readers
        for (int i = 0; i < Manager.readers.count; i++)
        {
            //  found ready file description
            if (FD_ISSET(currentFD, &readers_temp))
            {
                //  remove current FD from fd_set
                FD_CLR(currentFD, &Manager.readers.set);

                tempFD = listViewer->fd;
                int fdState = 0;
                if (tempFD.type == SOCKET_FD)
                {
                    fdState = GiveControlToSocket(&tempFD);
                }
                else 
                {
                    fdState = GiveControlToFile(&tempFD);
                }

                fdState++;

                processedFD++;
                if (processedFD == readyFD)
                {
                    break;
                }
            }
        }

        //     if ()
        //     {
        //         //  client ready to read from file
        //         struct Mail letter = ReadDataFromFile(0);
        //         SendMail(readyFD, letter);
        //         RevokeLetter(letter);
        //         continue;
        //     }

        //     if (FD_ISSET(readyFD, &Manager.writers.set))
        //     {
        //         //  client ready to send data
        //         continue;
        //     }

        //     if (FD_ISSET(readyFD, &Manager.handlers.set))
        //     {
        //         //  client receive interrupt
        //         break;
        //     }
        // }
    }
}

//  Stop work method
void Stop()
{
}

//  Dispose resource
void Dispose()
{
    DisposeFileViewer();
}

//==========================//
//      PRIVATE METHODS     //
//==========================//
