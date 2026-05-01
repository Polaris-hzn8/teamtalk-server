#ifndef __STRING_H__
#define __STRING_H__

#include <string>

// 字符串分割类
class CStrExplode {
 public:
  CStrExplode(char* str, char seperator);
  virtual ~CStrExplode();

  uint32_t GetItemCnt() { return m_item_cnt; }
  char* GetItem(uint32_t idx) { return m_item_list[idx]; }

 private:
  uint32_t m_item_cnt;  //子字符串数量
  char** m_item_list;   //子字符串
};

//将字符串进行URL编码
std::string URLEncode(const std::string& sIn);

//对URL编码的字符串进行解码
std::string URLDecode(const std::string& sIn);

//将数字转换为对应十六进制字符
/// @brief 将数字转换为对应十六进制字符
/// @param x 要转换的数字
/// @return 转换后的十六进制字符
inline unsigned char toHex(const unsigned char& x);

//将十六进制表示的字符转换为对应的数值
inline unsigned char fromHex(const unsigned char& x);

//在内存中查找子字符串
/// @brief 在内存中查找子字符串
/// @param src_str 源字符串
/// @param src_len 源字符串长度
/// @param sub_str 子字符串
/// @param sub_len 子字符串长度
/// @param flag 是否区分大小写
/// @return 子字符串在源字符串中的位置
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag = true);

#endif  // __STRING_H__