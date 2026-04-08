#ifndef _URL_CODEC_H_
#define _URL_CODEC_H_

#include <cstdint>
#include <string>

/// @brief 将id转换为url（线程安全）
/// @param id 要转换的id
/// @return 转换后的url
std::string idtourl(uint32_t id);

/// @brief 将url转换为id
/// @param url 要转换的url
/// @return 转换后的id
uint32_t urltoid(const char* url);

#endif  // _URL_CODEC_H_