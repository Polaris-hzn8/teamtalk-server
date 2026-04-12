
#include "string.h"

CStrExplode::CStrExplode(char* str, char seperator) {
  m_item_cnt = 1;
  char* pos = str;

  while (*pos) {
    if (*pos == seperator)
      m_item_cnt++;
    pos++;
  }

  m_item_list = new char*[m_item_cnt];

  int idx = 0;
  char* start = pos = str;
  while (*pos) {
    if (pos != start && *pos == seperator) {
      uint32_t len = pos - start;
      m_item_list[idx] = new char[len + 1];
      strncpy(m_item_list[idx], start, len);
      m_item_list[idx][len] = '\0';
      idx++;
      start = pos + 1;
    }
    pos++;
  }

  uint32_t len = pos - start;
  if (len != 0) {
    m_item_list[idx] = new char[len + 1];
    strncpy(m_item_list[idx], start, len);
    m_item_list[idx][len] = '\0';
  }
}

CStrExplode::~CStrExplode() {
  for (uint32_t i = 0; i < m_item_cnt; i++)
    delete[] m_item_list[i];
  delete[] m_item_list;
}

std::string URLEncode(const std::string& sIn) {
  std::string sOut;
  for (size_t ix = 0; ix < sIn.size(); ix++) {
    unsigned char buf[4];
    memset(buf, 0, 4);
    if (isalnum((unsigned char)sIn[ix])) {
      buf[0] = sIn[ix];
    } else {
      buf[0] = '%';
      buf[1] = toHex((unsigned char)sIn[ix] >> 4);
      buf[2] = toHex((unsigned char)sIn[ix] % 16);
    }
    sOut += (char*)buf;
  }
  return sOut;
}

std::string URLDecode(const std::string& sIn) {
  std::string sOut;
  for (size_t ix = 0; ix < sIn.size(); ix++) {
    unsigned char ch = 0;
    if (sIn[ix] == '%') {
      ch = (fromHex(sIn[ix + 1]) << 4);
      ch |= fromHex(sIn[ix + 2]);
      ix += 2;
    } else if (sIn[ix] == '+') {
      ch = ' ';
    } else {
      ch = sIn[ix];
    }
    sOut += (char)ch;
  }
  return sOut;
}

inline unsigned char toHex(const unsigned char& x) {
  return x > 9 ? x - 10 + 'A' : x + '0';
}

inline unsigned char fromHex(const unsigned char& x) {
  return isdigit(x) ? x - '0' : x - 'A' + 10;
}

// 内存中查找字符串
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag) {
  if (NULL == src_str || NULL == sub_str || src_len <= 0)
    return NULL;
  if (src_len < sub_len)
    return NULL;

  const char* p;
  if (sub_len == 0)
    sub_len = strlen(sub_str);
  if (sub_len == src_len) {
    if (0 == (memcmp(src_str, sub_str, src_len)))
      return src_str;
    else
      return NULL;
  }

  if (flag) {
    /* 从左向右搜索 */
    for (int i = 0; i < src_len - sub_len; i++) {
      p = src_str + i;
      if (0 == memcmp(p, sub_str, sub_len))
        return p;
    }
  } else {
    /* 从右向左搜索 */
    for (int i = (src_len - sub_len); i >= 0; i--) {
      p = src_str + i;
      if (0 == memcmp(p, sub_str, sub_len))
        return p;
    }
  }

  return NULL;
}