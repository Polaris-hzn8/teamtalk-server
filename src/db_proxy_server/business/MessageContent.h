/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: MessageContent.h
 Update Time: Thu 15 Jun 2023 00:31:41 CST
 brief:
*/

#ifndef MESSAGECOUTENT_H_
#define MESSAGECOUTENT_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void getMessage(CImPdu* pPdu, uint32_t conn_uuid);

void sendMessage(CImPdu* pPdu, uint32_t conn_uuid);

void getMessageById(CImPdu* pPdu, uint32_t conn_uuid);

void getLatestMsgId(CImPdu* pPdu, uint32_t conn_uuid);

};

#endif /* MESSAGECOUTENT_H_ */
