/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.cpp
 Update Time: Mon 12 Jun 2023 17:04:14 CST
 brief: 各种工具类封装
*/

#include <sstream>
#include "basic_tools.h"

uint64_t get_tick_count() {
#ifdef _MSC_VER
  LARGE_INTEGER liCounter;
  LARGE_INTEGER liCurrent;

  if (!QueryPerformanceFrequency(&liCounter))
    return GetTickCount();

  QueryPerformanceCounter(&liCurrent);
  return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
  struct timeval tval;
  uint64_t ret_tick;

  gettimeofday(&tval, NULL);

  ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
  return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond) {
#ifdef _MSC_VER
  Sleep(millisecond);
#else
  usleep(millisecond * 1000);
#endif
}

void writePid() {
  uint32_t curPid;
#ifdef _MSC_VER
  curPid = (uint32_t)GetCurrentProcess();
#else
  curPid = (uint32_t)getpid();
#endif
  FILE* f = fopen("server.pid", "w");
  assert(f);
  char szPid[32];
  sprintf_s(szPid, sizeof(szPid), "%d", curPid);
  fwrite(szPid, strlen(szPid), 1, f);
  fclose(f);
}

size_t get_file_size(const char* path) {
  size_t filesize = -1;
  struct stat statbuff;
  if (stat(path, &statbuff) < 0) {
    return filesize;
  } else {
    filesize = statbuff.st_size;
  }
  return filesize;
}

std::string idtourl(uint32_t id) {
  static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  // 加密偏移：乘2再加56，反向转换时用(number-56)>>1恢复
  uint32_t value = id * 2 + 56;

  char buf[64];
  char* ptr = buf + sizeof(buf) - 1;
  *ptr = '\0';

  do {
    *--ptr = digits[value % 36];
    value /= 36;
  } while (ptr > buf && value);

  // 添加版本号前缀 '1'
  *--ptr = '1';

  return std::string(ptr);
}

uint32_t urltoid(const char* url) {
  if (!url || url[0] == '\0') {
    return 0;
  }

  // 跳过版本号（第一个字符必须是 '1'）
  const char* p = url + 1;
  uint32_t number = 0;

  while (*p) {
    char c = *p++;
    uint32_t digit;

    if (c >= '0' && c <= '9') {
      digit = c - '0';
    } else if (c >= 'a' && c <= 'z') {
      digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'Z') {
      digit = c - 'A' + 10;
    } else {
      continue;
    }

    number = number * 36 + digit;
  }

  // 反向解密：减去偏移后除以2
  return (number - 56) >> 1;
}

char* long2ip(const unsigned int in) {
  unsigned short v1, v2, v3, v4;
  v4 = in % 256;
  v3 = (in >> 8) % 256;
  v2 = (in >> 16) % 256;
  v1 = (in >> 24) % 256;
  static char output[16];
  sprintf_s(output, sizeof(output), "%hd.%hd.%hd.%hd", v1, v2, v3, v4);
  return output;
}

unsigned int ip2long(const char* ip) {
  unsigned short v1, v2, v3, v4;
  sscanf_s(ip, "%hd.%hd.%hd.%hd", &v1, &v2, &v3, &v4);
  unsigned int vl = (v1 << 24) + (v2 << 16) + (v3 << 8) + v4;
  return vl;
}

std::string int2string(uint32_t user_id) {
  std::stringstream ss;
  ss << user_id;
  return ss.str();
}

uint32_t string2int(const std::string& value) {
  return (uint32_t)atoi(value.c_str());
}

void replace_mark(std::string& str, std::string& new_value, uint32_t& begin_pos) {
  std::string::size_type pos = str.find('?', begin_pos);
  if (pos == std::string::npos) {
    return;
  }

  std::string prime_new_value = "'" + new_value + "'";
  str.replace(pos, 1, prime_new_value);

  begin_pos = pos + prime_new_value.size();
}

void replace_mark(std::string& str, uint32_t new_value, uint32_t& begin_pos) {
  std::stringstream ss;
  ss << new_value;

  std::string str_value = ss.str();
  std::string::size_type pos = str.find('?', begin_pos);
  if (pos == std::string::npos) {
    return;
  }

  str.replace(pos, 1, str_value);
  begin_pos = pos + str_value.size();
}

char* replaceStr(char* pSrc, char oldChar, char newChar) {
  if (NULL == pSrc) {
    return NULL;
  }

  char* pHead = pSrc;
  while (*pHead != '\0') {
    if (*pHead == oldChar) {
      *pHead = newChar;
    }
    ++pHead;
  }
  return pSrc;
}
