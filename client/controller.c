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
    Manager.pointer = 0;

    InitSmtpSocket(& Manager.fdSet, & Manager.pointer, Manager.limit);
    InitFileViewer();
    
    int readyFD = -1;
    fd_set readSet;

    FD_ZERO(&readSet);
    for (int i = 0; i < Manager.pointer; i++) 
    {
        FD_SET(Manager.fdSet[i], &readSet);
    }


    
    while (1)
    {
        readyFD = pselect(Manager.pointer + 1, );
    }
}

//==========================//
//      PRIVATE METHODS     //
//==========================//

void allocateMemory(int count)
{
    Manager.fdSet = (int *)realloc(Manager.fdSet, sizeof(int) * count);
    if (Manager.fdSet == NULL)
    {
        printf("Failure of allocate memory for 'fdSet'");
        exit(-1);
    }

    Manager.limit = count;
    return;
}