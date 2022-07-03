/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#ifndef LFS_UTILITIES_H
#define LFS_UTILITIES_H

#include <FS.h>
#include <LittleFS.h>

#ifndef VERBOSE
#define VERBOSE             0
#endif

#define NUM_ITEMS(arr)  ((unsigned int)(sizeof(arr) / sizeof(arr[0])))


void formatLFS();

bool mountLFS();

void unmountLFS();

bool printFile(const String& path);

bool writeFile(const String& path, const char *contents);

bool appendFile(const String& path, const char *contents);

bool renameFile(const String& srcPath, const String &dstPath);

bool deleteFile(const String &path);

int getFiles(const String& dirName, char *paths);

void listFiles(const String& dirPath);

void listFilesLong(const String& dirPath, uint8_t indent=2);


#endif /*LFS_UTILITIES_H*/
