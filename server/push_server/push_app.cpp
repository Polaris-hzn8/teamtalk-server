/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: push_app.cpp
 Update Time: Sun 10 Aug 2025 12:11:12 CST
 brief:
*/

#include "push_app.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "config_file_reader.h"
#include "push_define.h"
#include "session_manager.h"
#include "slog_api.h"

CSLog g_pushlog = CSLog(LOG_MODULE_PUSH);

CPushApp::CPushApp() {
  m_bInit = FALSE;
}

CPushApp::~CPushApp() {}

CPushApp* CPushApp::GetInstance() {
  static CPushApp app;
  return &app;
}

BOOL CPushApp::Init() {
  if (!m_bInit) {
    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    // OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();

    m_bInit = TRUE;
    PUSH_SERVER_DEBUG("push app init successed.");
  } else {
    PUSH_SERVER_WARN("warning: push app has inited.");
  }

  return TRUE;
}

BOOL CPushApp::UnInit() {
  Stop();
  if (m_bInit) {
    m_bInit = FALSE;
    PUSH_SERVER_DEBUG("push app uninit successed.");
  } else {
    PUSH_SERVER_WARN("warning: push app has uninited.");
  }
  return TRUE;
}

BOOL CPushApp::Start() {
  if (m_bInit) {
    std::string file_name = "push_server.conf";
    CConfigFileReader config_file(file_name.c_str());
    if (!config_file.IsLoadSuccess()) {
      PUSH_SERVER_ERROR("push app config file: %s load failed.", file_name.c_str());
      return FALSE;
    }

    std::string listen_ip = config_file.GetConfigValue("ListenIP");
    std::string str_listen_port = config_file.GetConfigValue("ListenPort");
    std::string cert_path = config_file.GetConfigValue("CertPath");
    std::string key_path = config_file.GetConfigValue("KeyPath");
    std::string key_password = config_file.GetConfigValue("KeyPassword");
    std::string sand_box = config_file.GetConfigValue("SandBox");

    if (listen_ip.empty() || str_listen_port.empty() || cert_path.empty() || key_path.empty() || sand_box.empty() ||
        key_password.empty()) {
      PUSH_SERVER_ERROR(
        "push app config file: %s not exist or miss required parameter "
        "obtained.",
        file_name.c_str());
      return FALSE;
    }

    uint32_t nsand_box = config_file.GetUint32Value("SandBox", 0);
    if (nsand_box != 1 && nsand_box != 0) {
      PUSH_SERVER_ERROR("push app config parameter: sand_box has invaid value: %u.", nsand_box);
      return FALSE;
    }

    apns_client_ptr pAPNSClient(new CAPNSClient(m_io));
    pAPNSClient->SetCertPath(cert_path);
    pAPNSClient->SetKeyPath(key_path);
    pAPNSClient->SetKeyPassword(key_password);
    pAPNSClient->SetSandBox(static_cast<BOOL>(nsand_box));
    CSessionManager::GetInstance()->SetAPNSClient(pAPNSClient);

    push_server_ptr pPushServer(new CPushServer(m_io));
    pPushServer->SetListenIP(listen_ip);
    pPushServer->SetPort(config_file.GetIntValue("ListenPort", 0));
    CSessionManager::GetInstance()->SetPushServer(pPushServer);

    m_io.Start();
    CSessionManager::GetInstance()->StartCheckPushSession();
    if (pAPNSClient) {
      if (pAPNSClient->Start() == FALSE) {
        return FALSE;
      }
    }
    if (pPushServer) {
      if (pPushServer->Start() == FALSE) {
        return FALSE;
      }
    }
    PUSH_SERVER_DEBUG("push app start successed.");
  } else {
    PUSH_SERVER_WARN("push app not init before.");
  }

  return TRUE;
}

BOOL CPushApp::Stop() {
  if (m_bInit) {
    m_io.Stop();
    CSessionManager::GetInstance()->StopCheckPushSession();
    apns_client_ptr pAPNSClient = CSessionManager::GetInstance()->GetAPNSClient();
    if (pAPNSClient) {
      pAPNSClient->Stop();
    }
    push_server_ptr pPushServer = CSessionManager::GetInstance()->GetPushServer();
    if (pPushServer) {
      pPushServer->Stop();
    }
    CSessionManager::GetInstance()->StopAllPushSession();

    CSessionManager::GetInstance()->RemoveAPNSClient();
    CSessionManager::GetInstance()->RemovePushServer();
    CSessionManager::GetInstance()->ClearPushSession();
    PUSH_SERVER_DEBUG("push app stop successed.");
  } else {
    PUSH_SERVER_WARN("push app not init before.");
  }
  return TRUE;
}
