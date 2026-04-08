
#include "url_codec.h"
#include <cstring>
#include <string>

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