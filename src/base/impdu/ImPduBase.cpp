/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImPduBase.cpp
 Update Time: Mon 12 Jun 2023 23:49:04 CST
 brief:
*/

#include "ImPduBase.h"
#include "common/util.h"
#include "IM.BaseDefine.pb.h"
using namespace IM::BaseDefine;

CImPdu::CImPdu()
{
    m_pdu_header.version = IM_PDU_VERSION;
    m_pdu_header.flag = 0;
    m_pdu_header.service_id = SID_OTHER;
    m_pdu_header.command_id = 0;
    m_pdu_header.seq_num = 0;
    m_pdu_header.reversed = 0;
}

uchar_t* CImPdu::GetBuffer()
{
    return m_buf.GetBuffer();
}

uint32_t CImPdu::GetLength()
{
    return m_buf.GetWriteOffset();
}

uchar_t* CImPdu::GetBodyData()
{
    return m_buf.GetBuffer() + sizeof(PduHeader_t);
}

uint32_t CImPdu::GetBodyLength()
{
    uint32_t body_length = 0;
    body_length = m_buf.GetWriteOffset() - sizeof(PduHeader_t);
    return body_length;
}

void CImPdu::SetVersion(uint16_t version)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 4, version);
}

void CImPdu::SetFlag(uint16_t flag)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 6, flag);
}

void CImPdu::SetServiceId(uint16_t service_id)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 8, service_id);
}

void CImPdu::SetCommandId(uint16_t command_id)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 10, command_id);
}

void CImPdu::SetError(uint16_t error)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 12, error);
}

void CImPdu::SetSeqNum(uint16_t seq_num)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 12, seq_num);
}

void CImPdu::SetReversed(uint32_t reversed)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 14, reversed);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool CImPdu::IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len)
{
    if (len < IM_PDU_HEADER_LEN)
        return false;

    pdu_len = CByteStream::ReadUint32(buf);
    if (pdu_len < IM_PDU_HEADER_LEN) {
        throw CPduException(1, "pdu_len zero.");
        return false;
    }

    if (pdu_len > len) {
        // log("pdu_len=%d, len=%d\n", pdu_len, len);
        return false;
    }

    return true;
}

CImPdu* CImPdu::ReadPdu(uchar_t* buf, uint32_t len)
{
    uint32_t pdu_len = 0;
    if (!IsPduAvailable(buf, len, pdu_len))
        return NULL;

    uint16_t service_id = CByteStream::ReadUint16(buf + 8);
    uint16_t command_id = CByteStream::ReadUint16(buf + 10);
    
    CImPdu* pPdu = new CImPdu();
    if (new CImPdu()) {
        pPdu->Write(buf, pdu_len);                      // 消息体
        pPdu->ReadPduHeader(buf, IM_PDU_HEADER_LEN);    // 消息头
    }
    return pPdu;
}

void CImPdu::WriteHeader()
{
    uchar_t* buf = GetBuffer();
    // PDU长度
    CByteStream::WriteInt32(buf, GetLength());
    // PDU信息写入
    CByteStream::WriteUint16(buf + 4, m_pdu_header.version);        // 版本号
    CByteStream::WriteUint16(buf + 6, m_pdu_header.flag);           // 标志
    CByteStream::WriteUint16(buf + 8, m_pdu_header.service_id);     // 服务ID
    CByteStream::WriteUint16(buf + 10, m_pdu_header.command_id);    // 命令ID
    CByteStream::WriteUint16(buf + 12, m_pdu_header.seq_num);       // 包序号
    CByteStream::WriteUint16(buf + 14, m_pdu_header.reversed);      // 保留字段
}

int CImPdu::ReadPduHeader(uchar_t* buf, uint32_t len)
{
    int ret = -1;
    if (len >= IM_PDU_HEADER_LEN && buf) {
        /* 利用CByteStream读取二进制流式数据 */
        CByteStream tmpStream(buf, len);
        tmpStream >> m_pdu_header.length;
        tmpStream >> m_pdu_header.version;
        tmpStream >> m_pdu_header.flag;
        tmpStream >> m_pdu_header.service_id;
        tmpStream >> m_pdu_header.command_id;
        tmpStream >> m_pdu_header.seq_num;
        tmpStream >> m_pdu_header.reversed;
        ret = 0;
    }
    return ret;
}

// 将消息序列化为二进制数据 写入到缓冲区中
bool CImPdu::SetPBMsg(const google::protobuf::MessageLite* msg)
{
    m_buf.Read(NULL, m_buf.GetWriteOffset());
    m_buf.Write(NULL, sizeof(PduHeader_t));

    uint32_t msg_size = msg->ByteSize();
    uchar_t* szData = new uchar_t[msg_size];
    if (!msg->SerializeToArray(szData, msg_size))
        log_error("pb msg miss required fields.");

    m_buf.Write(szData, msg_size);
    delete[] szData;

    WriteHeader();
    return true;
}

