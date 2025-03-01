/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImPduBase.cpp
 Update Time: Mon 12 Jun 2023 23:49:04 CST
 brief:
*/

#include "ImPduBase.h"
#include "IM.BaseDefine.pb.h"
#include "util.h"
using namespace IM::BaseDefine;

CImPdu::CImPdu() {
    m_pdu_header.version = IM_PDU_VERSION;
    m_pdu_header.flag = 0;
    m_pdu_header.service_id = SID_OTHER;
    m_pdu_header.command_id = 0;
    m_pdu_header.seq_num = 0;
    m_pdu_header.reversed = 0;
}

///////////////////////////////////Get////////////////////////////////////////
uchar_t* CImPdu::GetBuffer() {
    //获取 m_buf 对应的内部缓冲区的指针uchar_t*
    return m_buf.GetBuffer();
}

uint32_t CImPdu::GetLength() {
    //当前缓冲区写入位置的偏移量
    return m_buf.GetWriteOffset();
}

uchar_t* CImPdu::GetBodyData() {
    //将内部缓冲区的指针向后偏移sizeof(PduHeader_t)字节，指向PDU消息体的起始位置
    return m_buf.GetBuffer() + sizeof(PduHeader_t);
}

uint32_t CImPdu::GetBodyLength() {
    uint32_t body_length = 0;
    //当前缓冲区写入位置的偏移量 - PDU 头部的长度
    body_length = m_buf.GetWriteOffset() - sizeof(PduHeader_t);
    return body_length;
}

////////////////////////////////////////////Set///////////////////////////////////////////
void CImPdu::SetVersion(uint16_t version) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 4, version);
}

void CImPdu::SetFlag(uint16_t flag) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 6, flag);
}

void CImPdu::SetServiceId(uint16_t service_id) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 8, service_id);
}

void CImPdu::SetCommandId(uint16_t command_id) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 10, command_id);
}

void CImPdu::SetError(uint16_t error) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 12, error);
}

void CImPdu::SetSeqNum(uint16_t seq_num) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 12, seq_num);
}

void CImPdu::SetReversed(uint32_t reversed) {
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + 14, reversed);
}

//////////////////////////////////////////////////////////////////////////////////////////


/**
 * 将 m_pdu_header 中的数据写入到内部的缓冲区 m_buf 的开头（即写入 PDU 的头部信息）
 * m_buf 主要用于存储 PDU 的数据内容，包括头部信息和消息体
*/
void CImPdu::WriteHeader() {
    // 1.获取内部缓冲区的指针
    uchar_t* buf = GetBuffer();

    // 2.使用CByteStream::WriteInt32将PDU的总长度写入 buf 的前4个字节
    CByteStream::WriteInt32(buf, GetLength());
    // 3.使用CByteStream::WriteUint16函数依次将 m_pdu_header 中的版本号、标志、服务ID、命令ID、包序号和保留字段写入 buf 中的相应位置
    CByteStream::WriteUint16(buf + 4, m_pdu_header.version);
    CByteStream::WriteUint16(buf + 6, m_pdu_header.flag);
    CByteStream::WriteUint16(buf + 8, m_pdu_header.service_id);
    CByteStream::WriteUint16(buf + 10, m_pdu_header.command_id);
    CByteStream::WriteUint16(buf + 12, m_pdu_header.seq_num);
    CByteStream::WriteUint16(buf + 14, m_pdu_header.reversed);
}

/// @brief 从给定的缓冲区中解析 PDU 的头部信息并将解析结果存储在 m_pdu_header 中
/// @param buf 缓冲区 
/// @param len 缓冲区长度
/// @return 
int CImPdu::ReadPduHeader(uchar_t* buf, uint32_t len) {
    int ret = -1;
    // 1.确保缓冲区长度len大于等于PDU头部长度 且缓冲区指针 buf 不为 nullptr
    if (len >= IM_PDU_HEADER_LEN && buf) {
        // 2.创建一个CByteStream对象is，将缓冲区buf作为参数传入构造函数，以便进行数据读取操作。
        CByteStream is(buf, len);
        // 3.使用CByteStream对象的重载的>>运算符，依次从is中读取PDU头部的各个字段
        // 将读取结果存储在 m_pdu_header 的对应成员变量中
        is >> m_pdu_header.length;
        is >> m_pdu_header.version;
        is >> m_pdu_header.flag;
        is >> m_pdu_header.service_id;
        is >> m_pdu_header.command_id;
        is >> m_pdu_header.seq_num;
        is >> m_pdu_header.reversed;

        ret = 0;
    }
    return ret;
}

/// @brief 从给定的缓冲区 buf 中解析出一个完整的 PDU，并返回一个新创建的 CImPdu 对象
/// @param buf 缓冲区
/// @param len 缓冲区长度
/// @return 
CImPdu* CImPdu::ReadPdu(uchar_t* buf, uint32_t len) {
    uint32_t pdu_len = 0;
    // 1.检查给定缓冲区中是否存在完整的 PDU 数据
    if (!IsPduAvailable(buf, len, pdu_len)) return NULL;

    // 2.从缓冲区 buf 中读取 service_id 和 command_id 字段，分别存储在对应的变量中
    uint16_t service_id = CByteStream::ReadUint16(buf + 8);
    uint16_t command_id = CByteStream::ReadUint16(buf + 10);
    
    CImPdu* pPdu = NULL;
    pPdu = new CImPdu();
    // 3.调用pPdu->Write(buf, pdu_len)将缓冲区buf中的数据写入到pPdu对象的内部缓冲区中（消息体）
    pPdu->Write(buf, pdu_len);
    
    // 4.调用pPdu->ReadPduHeader(buf, IM_PDU_HEADER_LEN)解析pPdu对象的PDU头部信息（消息头）
    pPdu->ReadPduHeader(buf, IM_PDU_HEADER_LEN);

    return pPdu;
}

/// @brief 检查给定的缓冲区buf中是否存在完整的PDU数据，并将解析出的PDU长度存储在pdu_len中
/// @param buf 缓冲区
/// @param len 缓冲区长度
/// @param pdu_len PDU长度
/// @return 
bool CImPdu::IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len) {
    // 1.缓冲区的长度是否小于 PDU 头部长度
    if (len < IM_PDU_HEADER_LEN) return false;

    // 2.从缓冲区buf中读取出PDU的长度，并将其存储在变量pdu_len中
    pdu_len = CByteStream::ReadUint32(buf);
    if (pdu_len > len) {
        //函数检查读取到的 PDU 长度是否超过了给定的缓冲区长度 len
        // log("pdu_len=%d, len=%d\n", pdu_len, len);
        return false;
    }

    // 3.读取到的PDU长度为0，即pdu_len为0，则抛出一个CPduException异常
    if (0 == pdu_len) throw CPduException(1, "pdu_len is 0");

    // 4.以上条件都满足，则表示存在完整的PDU数据返回true
    return true;
}

/// @brief 用于将给定的 Protobuf 消息（msg）设置为当前 PDU 对象的消息体（protobuf消息转化）
/// @param msg protobuf消息
void CImPdu::SetPBMsg(const google::protobuf::MessageLite* msg) {
    // 1.设置包体，则需要重置下空间
    // 清空内部缓冲区的数据，将读取偏移量重置为0，相当于清空缓冲区
    m_buf.Read(NULL, m_buf.GetWriteOffset());
    // 向内部缓冲区写入一个大小为 PduHeader_t 的空数据，相当于写入 PDU 头部占用的空间
    m_buf.Write(NULL, sizeof(PduHeader_t));

    // 2.获取 Protobuf 消息的大小
    uint32_t msg_size = msg->ByteSize();

    // 3.创建一个大小为msg_size的uchar_t数组，命名为szData用于存储序列化后的消息数据
    uchar_t* szData = new uchar_t[msg_size];

    // 4.将Protobuf消息序列化为二进制数据，并存储到szData数组中
    if (!msg->SerializeToArray(szData, msg_size)) log_error("pb msg miss required fields.");

    // 5.调用 m_buf.Write(szData, msg_size) 将序列化后的消息数据写入内部缓冲区
    m_buf.Write(szData, msg_size);

    // 6.释放 szData 数组的内存
    delete[] szData;

    // 7.调用WriteHeader()函数，将PDU头部信息写入内部缓冲区
    WriteHeader();
}


