#ifndef FILE_VIEWER_H
#define FILE_VIEWER_H

#include <string.h>
#include "structs.h"

void InitFileViewer();
struct Mail ReadDataFromFile(int fd);
void RevokeLetter(struct Mail letter);
int GiveControlToFile(struct FileDesc *fd);
void DisposeFileViewer();

#endif //FILE_VIEWER_H