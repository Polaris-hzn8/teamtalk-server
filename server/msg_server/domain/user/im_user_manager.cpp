/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImUserManager.cpp
 Update Time: Thu 15 Jun 2023 00:55:22 CST
 brief:
*/

#include <teamtalk/imcore/ttidl/login.pb.h>
#include <teamtalk/imcore/slog/slog.h>

#include "domain/user/im_user_manager.h"
#include "connection/msg_conn.h"
#include "connection/route_serv_conn.h"

using namespace std;

namespace teamtalk::msg_server::domain::user {

namespace ttconnection = teamtalk::msg_server::connection;
namespace ttidlbase = teamtalk::imcore::ttidl::base_define;
namespace ttidllogin = teamtalk::imcore::ttidl::login;

CImUserManager::~CImUserManager() {
  RemoveAll();
}

CImUserManager* CImUserManager::GetInstance() {
  static CImUserManager s_manager;
  return &s_manager;
}

CImUser* CImUserManager::GetImUserByLoginName(string login_name) {
  CImUser* pUser = nullptr;
  ImUserMapByName_t::iterator it = m_im_user_map_by_name.find(login_name);
  if (it != m_im_user_map_by_name.end()) {
    pUser = it->second;
  }
  return pUser;
}

CImUser* CImUserManager::GetImUserById(uint32_t user_id) {
  CImUser* pUser = nullptr;
  ImUserMap_t::iterator it = m_im_user_map.find(user_id);
  if (it != m_im_user_map.end()) {
    pUser = it->second;
  }
  return pUser;
}

ttconnection::CMsgConn* CImUserManager::GetMsgConnByHandle(uint32_t user_id, uint32_t handle) {
  ttconnection::CMsgConn* pMsgConn = nullptr;
  CImUser* pImUser = GetImUserById(user_id);
  if (pImUser) {
    pMsgConn = pImUser->GetMsgConn(handle);
  }
  return pMsgConn;
}

bool CImUserManager::AddImUserByLoginName(string login_name, CImUser* pUser) {
  bool bRet = false;
  if (GetImUserByLoginName(login_name) == nullptr) {
    m_im_user_map_by_name[login_name] = pUser;
    bRet = true;
  }
  return bRet;
}

void CImUserManager::RemoveImUserByLoginName(string login_name) {
  m_im_user_map_by_name.erase(login_name);
}

bool CImUserManager::AddImUserById(uint32_t user_id, CImUser* pUser) {
  bool bRet = false;
  if (GetImUserById(user_id) == nullptr) {
    m_im_user_map[user_id] = pUser;
    bRet = true;
  }
  return bRet;
}

void CImUserManager::RemoveImUserById(uint32_t user_id) {
  m_im_user_map.erase(user_id);
}

void CImUserManager::RemoveImUser(CImUser* pUser) {
  if (pUser != nullptr) {
    RemoveImUserById(pUser->GetUserId());
    RemoveImUserByLoginName(pUser->GetLoginName());
    delete pUser;
    pUser = nullptr;
  }
}

void CImUserManager::RemoveAll() {
  for (auto& kv : m_im_user_map_by_name) {
    if (kv.second) {
      delete kv.second;
      kv.second = nullptr;
    }
  }
  m_im_user_map_by_name.clear();
  m_im_user_map.clear();
}

void CImUserManager::GetOnlineUserInfo(list<user_stat_t>* online_user_info) {
  user_stat_t status;
  CImUser* pImUser = nullptr;
  for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++) {
    pImUser = (CImUser*)it->second;
    if (pImUser->IsValidate()) {
      map<uint32_t, ttconnection::CMsgConn*>& ConnMap = pImUser->GetMsgConnMap();
      for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = ConnMap.begin(); it != ConnMap.end(); it++) {
        ttconnection::CMsgConn* pConn = it->second;
        if (pConn->IsOpen()) {
          status.user_id = pImUser->GetUserId();
          status.client_type = pConn->GetClientType();
          status.status = pConn->GetOnlineStatus();
          online_user_info->push_back(status);
        }
      }
    }
  }
}

void CImUserManager::GetUserConnCnt(list<user_conn_t>* user_conn_list, uint32_t& total_conn_cnt) {
  total_conn_cnt = 0;
  CImUser* pImUser = nullptr;
  for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++) {
    pImUser = (CImUser*)it->second;
    if (pImUser->IsValidate()) {
      user_conn_t user_conn_cnt = pImUser->GetUserConn();
      user_conn_list->push_back(user_conn_cnt);
      total_conn_cnt += user_conn_cnt.conn_cnt;
    }
  }
}

void CImUserManager::BroadcastPdu(ttnetlib::CImPdu* pdu, uint32_t client_type_flag) {
  CImUser* pImUser = nullptr;
  for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++) {
    pImUser = (CImUser*)it->second;
    if (pImUser->IsValidate()) {
      switch (client_type_flag) {
        case CLIENT_TYPE_FLAG_PC:
          pImUser->BroadcastPduWithOutMobile(pdu);
          break;
        case CLIENT_TYPE_FLAG_MOBILE:
          pImUser->BroadcastPduToMobile(pdu);
          break;
        case CLIENT_TYPE_FLAG_BOTH:
          pImUser->BroadcastPdu(pdu);
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace teamtalk::msg_server::domain::user