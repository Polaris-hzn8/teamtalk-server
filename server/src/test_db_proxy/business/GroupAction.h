/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: GroupAction.h
 Update Time: Thu 15 Jun 2023 01:05:57 CST
 brief:
*/

#ifndef GROUPACTION_H_
#define GROUPACTION_H_

#include "ImPduBase.h"

namespace DB_PROXY {

void createGroup(CImPdu* pPdu, uint32_t conn_uuid);

void getNormalGroupList(CImPdu* pPdu, uint32_t conn_uuid);

void getGroupInfo(CImPdu* pPdu, uint32_t conn_uuid);

void modifyMember(CImPdu* pPdu, uint32_t conn_uuid);

void setGroupPush(CImPdu* pPdu, uint32_t conn_uuid);

void getGroupPush(CImPdu* pPdu, uint32_t conn_uuid);

};

#endif /* GROUPACTION_H_ */
