/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AttachData.cpp
 Update Time: Thu 15 Jun 2023 00:54:10 CST
 brief:
*/

#include "AttachData.h"

CDbAttachData::CDbAttachData(uint32_t type, uint32_t handle, uint32_t service_type /* = 0 */) // 序列化
{
    CByteStream os(&m_buf, 0);

    os << type;
    os << handle;
    os << service_type;
    wstring wstrIn;
}

CDbAttachData::CDbAttachData(uchar_t* attach_data, uint32_t attach_len) // 反序列化
{
    CByteStream is(attach_data, attach_len);

    is >> m_type;
    is >> m_handle;
    is >> m_service_type;
}

CPduAttachData::CPduAttachData(uint32_t type, uint32_t handle, uint32_t pduLength, uchar_t* pdu, uint32_t service_type) // 序列化
{
    CByteStream os(&m_buf, 0);

    os << type;
    os << handle;
    os << service_type;
    os.WriteData(pdu, pduLength);
}

CPduAttachData::CPduAttachData(uchar_t* attach_data, uint32_t attach_len) // 反序列化
{
    CByteStream is(attach_data, attach_len);

    is >> m_type;
    is >> m_handle;
    is >> m_service_type;
    m_pdu = is.ReadData(m_pduLength);
}