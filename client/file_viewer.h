#ifndef FILE_VIEWER_H
#define FILE_VIEWER_H

#include <string.h>
#include "structs.h"

void InitFileViewer();
struct Mail ReadDataFromFile(int fd);

#endif //FILE_VIEWER_H