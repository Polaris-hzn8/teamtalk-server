/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Login.h
 Update Time: Thu 15 Jun 2023 00:31:22 CST
 brief:
*/

#ifndef LOGIN_H_
#define LOGIN_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void doLogin(CImPdu* pPdu, uint32_t conn_uuid);

};

#endif /* LOGIN_H_ */
