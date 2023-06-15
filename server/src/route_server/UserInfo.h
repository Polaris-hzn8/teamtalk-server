/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UserInfo.h
 Update Time: Thu 15 Jun 2023 00:59:30 CST
 brief:
*/

#ifndef __im_server_TT__UserInfo__
#define __im_server_TT__UserInfo__

#include "ostype.h"
#include <iostream>
#include <map>
#include <set>
class CRouteConn;

using namespace std;
class CUserInfo {
public:
    CUserInfo();
    ~CUserInfo();

    uint32_t GetStatus();

    void AddRouteConn(CRouteConn* pConn) { m_RouteConnSet.insert(pConn); }
    void RemoveRouteConn(CRouteConn* pConn) { m_RouteConnSet.erase(pConn); }
    void ClearRouteConn() { m_RouteConnSet.clear(); }
    set<CRouteConn*>* GetRouteConn() { return &m_RouteConnSet; }
    bool FindRouteConn(CRouteConn* pConn);

    uint32_t GetRouteConnCount() { return m_RouteConnSet.size(); }
    void AddClientType(uint32_t client_type);
    void RemoveClientType(uint32_t client_type);
    uint32_t GetCountByClientType(uint32_t client_type);
    bool IsMsgConnNULL();
    void ClearClientType();

    bool IsPCClientLogin();
    bool IsMobileClientLogin();

private:
    set<CRouteConn*> m_RouteConnSet;
    map<uint32_t /* client_type */, uint32_t /* count */> m_ClientTypeList;
};

#endif /* defined(__im_server_TT__UserInfo__) */
