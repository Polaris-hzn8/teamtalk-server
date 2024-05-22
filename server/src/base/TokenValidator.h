/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: TokenValidator.h
 Update Time: Tue 13 Jun 2023 17:09:29 CST
 brief:
*/

#ifndef TOKENVALIDATOR_H_
#define TOKENVALIDATOR_H_

#include "util.h"

/// @brief 用于生成令牌token
/// @param uid 用户id
/// @param time_offset 时间偏移量
/// @param md5_str_buf 指向字符数组的指针
/// @return 0 if generate token successful
int genToken(unsigned int uid, time_t time_offset, char* md5_str_buf);

/// @brief 验证用户令牌token是否有效
/// @param user_id 用户id
/// @param token 
/// @return 
bool IsTokenValid(uint32_t user_id, const char* token);

#endif
