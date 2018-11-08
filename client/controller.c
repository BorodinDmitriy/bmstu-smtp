#include "controller.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//
struct Controller Manager;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitController()
{
    FD_ZERO(&Manager.writers.set);
    Manager.writers.count = 0;

    FD_ZERO(&Manager.readers.set);
    Manager.readers.count = 0;

    InitSmtpSocket(&Manager.writers);
    InitFileViewer(&Manager.readers);

    return;
}

void Run()
{
    int readyFD = -1;
    struct timespec timer_spec;
    timer_spec.tv_sec = 10;
    timer_spec.tv_nsec = 0;

    while (1)
    {
        int n = Manager.writers.count + Manager.readers.count + Manager.handlers.count + 1;
        readyFD = pselect(n, &Manager.readers.set, &Manager.writers.set, &Manager.handlers.set, &timer_spec, NULL);

        if (readyFD <= 0)
        {
            struct Mail letter = ReadDataFromFile(00);
            SendMail(readyFD, letter);
            continue;
        }

        printf("ready new data\n");

        if (FD_ISSET(readyFD, &Manager.readers.set))
        {
            //  client ready to read from file
            struct Mail letter = ReadDataFromFile(0);
            SendMail(readyFD, letter);
            RevokeLetter(letter);
            continue;
        }

        if (FD_ISSET(readyFD, &Manager.writers.set))
        {
            //  client ready to send data
            continue;
        }

        if (FD_ISSET(readyFD, &Manager.handlers.set))
        {
            //  client receive interrupt
            break;
        }
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
    DisposeSmtpSockets();   
}

//==========================//
//      PRIVATE METHODS     //
//==========================//
