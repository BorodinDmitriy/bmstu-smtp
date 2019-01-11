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
void DestroyTask(struct worker_task *task);
void SearchNewFiles();
int MoveLetter(char *from, char *to);
int SetPathInNewDirectory(char *dest, char *path);
int SetPathInCurrentDirectory(char *dest, char *path);

#endif //FILE_VIEWER_H