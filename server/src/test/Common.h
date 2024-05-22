/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Common.h
 Update Time: Thu 15 Jun 2023 01:01:24 CST
 brief:
*/

#ifndef __COMMON_H__
#define __COMMON_H__
#include "ImPduBase.h"

#define PROMPT "im-client> "
#define PROMPTION fprintf(stderr, "%s", PROMPT);

typedef void (*packet_callback_t)(CImPdu* pPdu);

#endif /*defined(__COMMON_H__) */
