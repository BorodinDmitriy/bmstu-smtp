#include "file_viewer.h"

void InitFileViewer() 
{

}

struct Mail ReadDataFromFile(int fd) 
{
    //  fake
    struct Mail letter = { (char *)calloc(100, sizeof(char)), 100, (char *)calloc(100, sizeof(char)), 100, (char *)calloc(100, sizeof(char)), 100};

    letter.from = "IU7.2@yandex.ru";
    letter.to = "IU7.2@yandex.ru";
    letter.message = "Test SMTP";

    return letter;
}