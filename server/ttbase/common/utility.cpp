#include "utility.h"
#include <sys/stat.h>

int64_t get_file_size(const char* path) {
  int64_t filesize = -1;
  struct stat statbuff;
  if (stat(path, &statbuff) < 0) {
    return filesize;  //获取文件信息失败
  } else {
    filesize = statbuff.st_size;
  }
  return filesize;
}