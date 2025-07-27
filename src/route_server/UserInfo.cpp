/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UserInfo.cpp
 Update Time: Thu 15 Jun 2023 00:59:24 CST
 brief:
*/

#include "UserInfo.h"
#include "ImPduBase.h"
#include "public_define.h"
#include "IM.BaseDefine.pb.h"
using namespace IM::BaseDefine;

CUserInfo::CUserInfo()
{
}

CUserInfo::~CUserInfo()
{
}

void CUserInfo::AddClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.find(client_type);
    if (it != m_clientTypeList.end()) {
        it->second += 1;
    } else {
        m_clientTypeList[client_type] = 1;
    }
}

void CUserInfo::RemoveClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.find(client_type);
    if (it != m_clientTypeList.end()) {
        uint32_t count = it->second;
        count -= 1;
        if (count > 0) {
            it->second = count;
        } else {
            m_clientTypeList.erase(client_type);
        }
    }
}

bool CUserInfo::FindRouteConn(CRouteConn* pConn)
{
    set<CRouteConn*>::iterator it = m_RouteConnSet.find(pConn);
    if (it != m_RouteConnSet.end()) {
        return true;
    } else {
        return false;
    }
}

uint32_t CUserInfo::GetCountByClientType(uint32_t client_type)
{
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.find(client_type);
    if (it != m_clientTypeList.end()) {
        return it->second;
    } else {
        return 0;
    }
}

bool CUserInfo::IsMsgConnNULL()
{
    if (m_clientTypeList.size() == 0) {
        return true;
    } else {
        return false;
    }
}

void CUserInfo::ClearClientType()
{
    m_clientTypeList.clear();
}

bool CUserInfo::IsPCClientLogin()
{
    bool bRet = false;
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.begin();
    for (; it != m_clientTypeList.end(); it++) {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_PC(client_type)) {
            bRet = true;
            break;
        }
    }
    return bRet;
}

bool CUserInfo::IsMobileClientLogin()
{
    bool bRet = false;
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.begin();
    for (; it != m_clientTypeList.end(); it++) {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_MOBILE(client_type)) {
            bRet = true;
            break;
        }
    }
    return bRet;
}

uint32_t CUserInfo::GetStatus()
{
    uint32_t status = USER_STATUS_OFFLINE;
    map<uint32_t, uint32_t>::iterator it = m_clientTypeList.begin();
    for (; it != m_clientTypeList.end(); it++) {
        uint32_t client_type = it->first;
        if (CHECK_CLIENT_TYPE_PC(client_type)) {
            status = USER_STATUS_ONLINE;
            break;
        }
    }
    return status;
}