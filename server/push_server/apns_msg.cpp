/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: apns_msg.cpp
 Update Time: Sun 10 Aug 2025 12:07:15 CST
 brief:
*/

#include "apns_msg.h"
#include <arpa/inet.h>
#include <string.h>
#include <json/json.h>
#include "byte_stream.h"
#include "push_define.h"

CAPNSGateWayMsg::CAPNSGateWayMsg() {
  memset(&m_stHead, 0, sizeof(ST_GATEWAY_HEAD));
  __SetHeadLength(sizeof(ST_GATEWAY_HEAD));
  m_stHead.command_id = (uchar_t)2;
  m_nExpirationDate = 1;
  m_cPriority = APNS_PRIORITY_IMMEDIATELY;
  m_bSound = TRUE;
  m_nBadge = 0;
  m_nNotificationID = 0;
}

CAPNSGateWayMsg::~CAPNSGateWayMsg() {}

void CAPNSGateWayMsg::WriteHead() {
  m_stHead.frame_length = htonl(GetBodyLength());
  char* buf = m_databuffer.GetBuffer();
  memcpy(buf, &m_stHead, sizeof(ST_GATEWAY_HEAD));
}

BOOL CAPNSGateWayMsg::SerializeToArray() {
  BOOL bRet = FALSE;
  if (m_databuffer.GetWriteOffset() != 0) {
    PUSH_SERVER_WARN("push msg serialize failed, databuffer offset: %d.", m_databuffer.GetWriteOffset());
    return bRet;
  }
  if (m_strDeviceToken.length() != APNS_DEVICE_TOKEN_HEX_LENGTH) {
    PUSH_SERVER_WARN("push msg serialize failed, device token length: %d, token: %s.",
                     m_strDeviceToken.length(),
                     m_strDeviceToken.c_str());
    return bRet;
  }

  string strPayload = _BuildPayload();
  if (strPayload.length() > APNS_PAY_LOAD_MAX_LENGTH || strPayload.length() == 0) {
    PUSH_SERVER_WARN("push msg serialize failed, payload length: %d.", strPayload.length());
    return bRet;
  }

  m_databuffer.Write(NULL, GetHeadLength());

  // device token
  int8_t nItemID = APNS_ITEM_DEVICE_TOKEN;
  int16_t nItemDataLength = htons(APNS_DEVICE_TOKEN_BINARY_LENGTH);
  m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
  m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
  char szDeviceToken[APNS_DEVICE_TOKEN_HEX_LENGTH + 1] = {0};
  strcpy(szDeviceToken, m_strDeviceToken.c_str());
  int8_t device_token[APNS_DEVICE_TOKEN_BINARY_LENGTH] = {0};
  for (uint32_t i = 0, j = 0; i < APNS_DEVICE_TOKEN_BINARY_LENGTH; i++, j += 2) {
    int8_t binary = 0;
    char tmp[3] = {szDeviceToken[j], szDeviceToken[j + 1], '\0'};
    sscanf(tmp, "%x", &binary);
    device_token[i] = binary;
  }
  m_databuffer.Write((const char*)&device_token, sizeof(device_token));

  // pay load
  nItemID = APNS_ITEM_PAY_LOAD;
  nItemDataLength = htons(strPayload.length());
  m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
  m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
  m_databuffer.Write(strPayload.c_str(), (int32_t)strPayload.length());

  // notification ID
  nItemID = APNS_ITEM_NOTIFICATION_ID;
  nItemDataLength = htons(sizeof(m_nNotificationID));
  int32_t nNotificationID = htonl(m_nNotificationID);
  m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
  m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
  m_databuffer.Write((const char*)&nNotificationID, sizeof(nNotificationID));

  // expiration date
  nItemID = APNS_ITEM_EXPIRATION_DATE;
  nItemDataLength = htons(sizeof(m_nExpirationDate));
  int32_t nExpirationDate = htonl(m_nExpirationDate);
  m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
  m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
  m_databuffer.Write((const char*)&nExpirationDate, sizeof(nExpirationDate));

  // priority
  nItemID = APNS_ITEM_PRIORITY;
  nItemDataLength = htons(sizeof(m_cPriority));
  m_databuffer.Write((const char*)&nItemID, sizeof(nItemID));
  m_databuffer.Write((const char*)&nItemDataLength, sizeof(nItemDataLength));
  m_databuffer.Write((const char*)&m_cPriority, sizeof(m_cPriority));

  __SetTailLength(0);
  __SetBodyLength(m_databuffer.GetWriteOffset() - GetHeadLength() - GetTailLength());
  WriteHead();
  bRet = TRUE;
  PUSH_SERVER_DEBUG("push msg buffer length: %d, payload length: %d.", GetDataBufferLength(), strPayload.length());
  return bRet;
}

string CAPNSGateWayMsg::_BuildPayload() {
  Json::Value payload(Json::objectValue);
  Json::Value aps(Json::objectValue);
  Json::Value alert(Json::objectValue);

  if (!GetAlterBody().empty()) {
    alert["body"] = GetAlterBody();
  }
  if (!GetActionLocKey().empty()) {
    alert["action-loc-key"] = GetActionLocKey();
  }
  if (!GetLocKey().empty()) {
    alert["loc-key"] = GetLocKey();
  }
  if (!GetLocArgsList().empty()) {
    Json::Value loc_args(Json::arrayValue);
    const list<string>& loc_args_list = GetLocArgsList();
    for (list<string>::const_iterator it = loc_args_list.begin(); it != loc_args_list.end(); ++it) {
      loc_args.append(*it);
    }
    alert["loc-args"] = loc_args;
  }
  if (!GetLaunchImage().empty()) {
    alert["launch-image"] = GetLaunchImage();
  }

  if (GetSound() == FALSE) {
    // TODO: 静音推送：当前逻辑仅设置 badge
    aps["badge"] = GetBadge();
  } else {
    aps["alert"] = alert;
    aps["badge"] = GetBadge();
    aps["sound"] = "bingbong.aiff";
  }

  payload["aps"] = aps;
  payload["custom"] = GetCustomData();

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  const std::string json = Json::writeString(builder, payload);
  PUSH_SERVER_DEBUG("%s", json.c_str());
  return json;
}

CAPNSGateWayResMsg::CAPNSGateWayResMsg() {
  __SetHeadLength(1);
  __SetBodyLength(5);
  __SetTailLength(0);
  m_CommandID = 0;
  m_Status = 0;
  m_NotificationID = 0;
}

CAPNSGateWayResMsg::~CAPNSGateWayResMsg() {}

BOOL CAPNSGateWayResMsg::CheckMsgAvailable() {
  BOOL bRet = FALSE;
  if (m_databuffer.GetWriteOffset() >= GetDataLength()) {
    bRet = TRUE;
  }
  return bRet;
}

BOOL CAPNSGateWayResMsg::ParseFromArray(const char* buf, uint32_t len) {
  BOOL bRet = FALSE;
  if (m_databuffer.GetWriteOffset() != 0) {
    return bRet;
  }
  Append(buf, len);
  if (CheckMsgAvailable()) {
    memcpy(&m_CommandID, (void*)buf, sizeof(m_CommandID));
    memcpy(&m_Status, (void*)(buf + sizeof(m_CommandID)), sizeof(m_Status));
    memcpy(&m_NotificationID, (void*)(buf + sizeof(m_CommandID) + sizeof(m_Status)), sizeof(m_NotificationID));
    m_NotificationID = ntohl(m_NotificationID);
    bRet = TRUE;
  }
  return bRet;
}

CAPNSFeedBackResMsg::CAPNSFeedBackResMsg() {
  __SetHeadLength(APNS_FEEDBACK_MSG_TIME_LENGTH + APNS_FEEDBACK_MSG_TOKEN_LENGTH);
  __SetBodyLength(APNS_FEEDBACK_MSG_TOKEN);
  __SetTailLength(0);
  m_Time = 0;
  m_TokenLength = 0;
}

CAPNSFeedBackResMsg::~CAPNSFeedBackResMsg() {}

BOOL CAPNSFeedBackResMsg::CheckMsgAvailable() {
  BOOL bRet = FALSE;
  if (m_databuffer.GetWriteOffset() >= GetDataLength()) {
    bRet = TRUE;
  } else {
    PUSH_SERVER_INFO("CheckMsgAvailable error.");
  }
  return bRet;
}

BOOL CAPNSFeedBackResMsg::ParseFromArray(const char* buf, uint32_t len) {
  BOOL bRet = FALSE;
  if (m_databuffer.GetWriteOffset() != 0) {
    PUSH_SERVER_INFO("ParseFromArray error, GetWriteOffset.");
    return bRet;
  }
  Append(buf, len);
  if (CheckMsgAvailable()) {
    memcpy(&m_Time, (void*)buf, sizeof(m_Time));
    m_Time = ntohl(m_Time);
    memcpy(&m_TokenLength, (void*)(buf + sizeof(m_Time)), sizeof(m_TokenLength));
    m_TokenLength = ntohs(m_TokenLength);
    uchar_t binary_token[32] = {0};
    char device_token[APNS_DEVICE_TOKEN_HEX_LENGTH + 1] = {0};
    char* p = device_token;
    memcpy(binary_token, buf + sizeof(m_Time) + sizeof(m_TokenLength), m_TokenLength);
    //需要换算成16进制的字符串表示
    for (uint32_t i = 0; i < APNS_DEVICE_TOKEN_BINARY_LENGTH; i++) {
      snprintf(p, 3, "%2.2hhX", binary_token[i]);
      p += 2;
    }
    m_Token = device_token;
    bRet = TRUE;
  } else {
    PUSH_SERVER_INFO("CheckMsgAvailable error 2.0");
  }
  return bRet;
}