/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: FileAction.h
 Update Time: Thu 15 Jun 2023 01:05:30 CST
 brief:
*/

#ifndef __FILEACTION_H__
#define __FILEACTION_H__

#include "ImPduBase.h"

namespace DB_PROXY {
void hasOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
void addOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
void delOfflineFile(CImPdu* pPdu, uint32_t conn_uuid);
};

#endif /*defined(__FILEACTION_H__) */
