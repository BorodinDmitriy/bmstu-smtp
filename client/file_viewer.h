#ifndef FILE_VIEWER_H
#define FILE_VIEWER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <semaphore.h>
#include "structs.h"


#define LETTER_FRAME_SIZE 200
struct files_record
{
    char path [256];
    struct files_record * next;
};

int InitFileViewer();
void SearchNewFiles();
struct Mail ReadDataFromFile(int fd);
void RevokeLetter(struct Mail letter);
int GiveControlToFile(struct FileDesc *fd);
void DisposeFileViewer();

#endif //FILE_VIEWER_H