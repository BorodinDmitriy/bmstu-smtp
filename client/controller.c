#include "controller.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//
struct Controller Manager;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//
void allocateMemory(int);

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitController()
{
    allocateMemory(INIT_FD_SET_COUNT);

    FD_ZERO(&Manager.writers.set);
    Manager.writers.count = 0;

    FD_ZERO(&Manager.readers.set);
    Manager.readers.count = 0;

    InitSmtpSocket(& Manager.writers);
    InitFileViewer(& Manager.readers);
    
    int readyFD = -1;
    struct timespec timer;
    timer.tv_sec = 10;
    timer.tv_nsec = 0;
    
    while (1)
    {
        int n = Manager.writers.count + Manager.readers.count + Manager.handlers.count + 1;
        readyFD = pselect(n, Manager.readers.set, Manager.writers.set, timer, NULL);

        if (readyFD < 0) 
        {
            continue;
        }

        printf("ready new data\n");

        if (FD_ISSET(readyFD, &Manager.readers.set)) 
        {
            //  client ready to read from file

            continue;
        }

        if (FD_ISSET(readyFD, &Manager.writers.set)) 
        {
            //  client ready to send data 

            continue;
        }

        if (FD_ISSET(readyFD, &Manager.handlers.set)) 
        {
            //  client receive interupt

            break;
        }
        
    }
}

//==========================//
//      PRIVATE METHODS     //
//==========================//

void allocateMemory(int count)
{
//     Manager.fdSet = (int *)realloc(Manager.fdSet, sizeof(int) * count);
//     if (Manager.fdSet == NULL)
//     {
//         printf("Failure of allocate memory for 'fdSet'");
//         exit(-1);
//     }

//     Manager.limit = count;
    return;
}