/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#ifndef LFS_UTILITIES_H
#define LFS_UTILITIES_H

#include <FS.h>
#include <LittleFS.h>

#define VERBOSE             1
#define MAX_FILENAME_LEN    32

#define NUM_ITEMS(arr)  ((unsigned int)(sizeof(arr) / sizeof(arr[0])))


void formatLFS();

bool mountLFS();

void unmountLFS();

void listDir(const char *dirname);

bool printFile(const char *path);

bool writeFile(const char *path, const char *contents);

bool appendFile(const char *path, const char *contents);

bool renameFile(const char *srcPath, const char *dstPath);

bool deleteFile(const char *path);

//void listFiles(const String& dirPath = "/");

int getFiles(const char *dirname, char *paths);


#endif /*LFS_UTILITIES_H*/
