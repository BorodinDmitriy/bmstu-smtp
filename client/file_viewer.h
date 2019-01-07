#ifndef FILE_VIEWER_H
#define FILE_VIEWER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <semaphore.h>
#include "dictionary.h"
#include "structs.h"
#include "logger.h"


#define LETTER_FRAME_SIZE 200
struct files_record
{
    char path [256];
    struct files_record * next;
};

int InitFileViewer();
void DestroyFileViewer();
void SearchNewFiles();

#endif //FILE_VIEWER_H