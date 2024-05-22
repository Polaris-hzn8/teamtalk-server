/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DepartAction.h
 Update Time: Thu 15 Jun 2023 01:04:56 CST
 brief:
*/

#ifndef __DEPARTACTION_H__
#define __DEPARTACTION_H__
#include "ImPduBase.h"

namespace DB_PROXY {

void getChgedDepart(CImPdu* pPdu, uint32_t conn_uuid);
};

#endif /*defined(__DEPARTACTION_H__) */
