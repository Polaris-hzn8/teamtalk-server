/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AttachData.cpp
 Update Time: Thu 15 Jun 2023 00:41:57 CST
 brief:
*/

#include "connection/attach_data.h"

namespace teamtalk::http_server::connection {

CDbAttachData::CDbAttachData(uint32_t type, uint32_t handle, uint32_t service_type) {
  ttnetlib::CByteStream os(&m_buf, 0);
  os << type;
  os << handle;
  os << service_type;
}

CDbAttachData::CDbAttachData(uchar_t* attach_data, uint32_t attach_len) {
  ttnetlib::CByteStream is(attach_data, attach_len);
  is >> m_type;
  is >> m_handle;
  is >> m_service_type;
}

// 序列化
CPduAttachData::CPduAttachData(uint32_t type, uint32_t handle, uint32_t pduLength, uchar_t* pdu, uint32_t service_type) {
  ttnetlib::CByteStream os(&m_buf, 0);
  os << type;
  os << handle;
  os << service_type;
  os.WriteData(pdu, pduLength);
}

// 反序列化
CPduAttachData::CPduAttachData(uchar_t* attach_data, uint32_t attach_len) {
  ttnetlib::CByteStream is(attach_data, attach_len);
  is >> m_type;
  is >> m_handle;
  is >> m_service_type;
  m_pdu = is.ReadData(m_pduLength);
}

}  // namespace teamtalk::http_server::connection