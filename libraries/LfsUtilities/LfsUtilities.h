/***************************************************************************
 *
 * LittleFS Support Utilities Library
 * 
 ***************************************************************************/

#ifndef LFS_UTILITIES_H
#define LFS_UTILITIES_H

#define CS_USE_LITTLEFS     true
#include <ConfigStorage.h>


#define NUM_ITEMS(arr)  ((unsigned int)(sizeof(arr) / sizeof(arr[0])))


void listFiles(String dirPath);

void formatLFS();


#endif /*LFS_UTILITIES_H*/
