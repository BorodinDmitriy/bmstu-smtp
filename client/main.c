#include <stdio.h>
#include "controller.h"

int main(int argc, char *argv[])
{
    InitController();
    WatchMailDir();
    return 0;
}