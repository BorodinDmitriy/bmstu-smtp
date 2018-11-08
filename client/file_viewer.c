#include "file_viewer.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitFileViewer()
{
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

//==========================//
//      PRIVATE METHODS     //
//==========================//