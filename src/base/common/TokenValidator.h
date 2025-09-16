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

// 生成令牌token
int genToken(unsigned int uid, time_t time_offset, char* md5_str_buf);
// 验证令牌token
bool IsTokenValid(uint32_t user_id, const char* token);

#endif
