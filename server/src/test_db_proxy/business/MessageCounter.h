/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: MessageCounter.h
 Update Time: Thu 15 Jun 2023 01:07:19 CST
 brief:
*/

#ifndef MESSAGECOUNTER_H_
#define MESSAGECOUNTER_H_

#include "ImPduBase.h"
namespace DB_PROXY {

void getUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);
void clearUnreadMsgCounter(CImPdu* pPdu, uint32_t conn_uuid);

void setDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
void getDevicesToken(CImPdu* pPdu, uint32_t conn_uuid);
};

#endif /* MESSAGECOUNTER_H_ */
