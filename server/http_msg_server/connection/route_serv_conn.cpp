/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: RouteServConn.cpp
 Update Time: Thu 15 Jun 2023 00:43:04 CST
 brief:
*/

#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/slog/slog.h>
#include <teamtalk/imcore/netlib/core/im_pdu.h>

#include <teamtalk/imcore/ttidl/base_define.pb.h>
#include <teamtalk/imcore/ttidl/other.pb.h>
#include <teamtalk/imcore/ttidl/service.pb.h>

#include "connection/route_serv_conn.h"
#include "common/http/http_conn.h"
#include "common/http/http_pdu.h"

namespace teamtalk::http_server::connection {

namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttcommon = teamtalk::imcore::common;
namespace ttserverinfo = teamtalk::sbase::server_info;
namespace ttidlbase = teamtalk::imcore::ttidl::base_define;
namespace ttidlother = teamtalk::imcore::ttidl::other;
namespace ttidlserver = teamtalk::imcore::ttidl::service;

static ttnetlib::ConnMap_t g_route_server_conn_map;

static ttserverinfo::serv_info_t* g_route_server_list;
static uint32_t g_route_server_count;
static CRouteServConn* g_master_rs_conn = NULL;

void route_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  ttnetlib::ConnMap_t::iterator it_old;
  CRouteServConn* pConn = NULL;
  uint64_t cur_time = ttcommon::get_tick_count();

  for (ttnetlib::ConnMap_t::iterator it = g_route_server_conn_map.begin(); it != g_route_server_conn_map.end();) {
    it_old = it;
    it++;

    pConn = (CRouteServConn*)it_old->second;
    pConn->OnTimer(cur_time);
  }

  // reconnect RouteServer
  ttserverinfo::serv_check_reconnect<CRouteServConn>(g_route_server_list, g_route_server_count);
}

void init_route_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count) {
  g_route_server_list = server_list;
  g_route_server_count = server_count;

  ttserverinfo::serv_init<CRouteServConn>(g_route_server_list, g_route_server_count);

  ttnetlib::netlib_register_timer(route_server_conn_timer_callback, NULL, 1000);
}

bool is_route_server_available() {
  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen()) {
      return true;
    }
  }

  return false;
}

void send_to_all_route_server(ttnetlib::CImPdu* pPdu) {
  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen()) {
      pConn->SendPdu(pPdu);
    }
  }
}

// get the oldest route server connection
CRouteServConn* get_route_serv_conn() {
  return g_master_rs_conn;
}

void update_master_route_serv_conn() {
  uint64_t oldest_connect_time = (uint64_t)-1;
  CRouteServConn* pOldestConn = NULL;

  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time)) {
      pOldestConn = pConn;
      oldest_connect_time = pConn->GetConnectTime();
    }
  }

  g_master_rs_conn = pOldestConn;

  if (g_master_rs_conn) {
    ttidlserver::IMRoleSet msg;
    msg.set_master(1);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_ROLE_SET);
    g_master_rs_conn->SendPdu(&pdu);
  }
}

CRouteServConn::CRouteServConn() {
  m_bOpen = false;
  m_serv_idx = 0;
}

CRouteServConn::~CRouteServConn() {}

void CRouteServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx) {
  log_info("Connecting to RouteServer %s:%d ", server_ip, server_port);

  m_serv_idx = idx;
  m_handle = ttnetlib::netlib_connect(server_ip, server_port, ttnetlib::imconn_callback, (void*)&g_route_server_conn_map);

  if (m_handle != NETLIB_INVALID_HANDLE) {
    g_route_server_conn_map.insert(std::make_pair(m_handle, this));
  }
}

void CRouteServConn::Close() {
  ttserverinfo::serv_reset<CRouteServConn>(g_route_server_list, g_route_server_count, m_serv_idx);

  m_bOpen = false;
  if (m_handle != NETLIB_INVALID_HANDLE) {
    ttnetlib::netlib_close(m_handle);
    g_route_server_conn_map.erase(m_handle);
  }

  ReleaseRef();

  if (g_master_rs_conn == this) {
    update_master_route_serv_conn();
  }
}

void CRouteServConn::OnConfirm() {
  log_info("connect to route server success ");
  m_bOpen = true;
  m_connect_time = ttcommon::get_tick_count();
  g_route_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

  if (g_master_rs_conn == NULL) {
    update_master_route_serv_conn();
  }
}

void CRouteServConn::OnClose() {
  log_info("onclose from route server handle=%d ", m_handle);
  Close();
}

void CRouteServConn::OnTimer(uint64_t curr_tick) {
  if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
    ttidlother::IMHeartBeat msg;
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_HEARTBEAT);
    SendPdu(&pdu);
  }

  if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
    log_info("conn to route server timeout ");
    Close();
  }
}

void CRouteServConn::HandlePdu(ttnetlib::CImPdu* pPdu) {}

}  // namespace teamtalk::http_server::connection