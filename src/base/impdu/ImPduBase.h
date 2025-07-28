/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImPduBase.h
 Update Time: Mon 12 Jun 2023 23:48:57 CST
 brief: 通信数据包读取与解析
*/

#ifndef _IMPDUBASE_H_
#define _IMPDUBASE_H_

#include "UtilPdu.h"
#include "pb/google/protobuf/message_lite.h"

#define IM_PDU_HEADER_LEN		16
#define IM_PDU_VERSION			1

// 判空检查
#define ALLOC_FAIL_ASSERT(p) \
    if (p == NULL) \
        throw CPduException(m_pdu_header.service_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed");

// 结果检查
#define CHECK_PB_PARSE_MSG(ret) {\
    if (ret == false) {\
        log_error("parse pb msg failed.");\
        return;\
    }\
}

#ifdef WIN32
    #ifdef BUILD_PDU
        #define DLL_MODIFIER __declspec(dllexport)
    #else
        #define DLL_MODIFIER __declspec(dllimport)1
    #endif
#else
    #define DLL_MODIFIER
#endif

typedef struct {
    uint32_t 	length;     // 协议包长度
    uint16_t 	version;    // 协议版本号
    uint16_t	flag;       // 未使用
    uint16_t	service_id; // 服务id
    uint16_t	command_id; // 命令id
    uint16_t	seq_num;    // 包序号
    uint16_t    reversed;   // 预留字段
} PduHeader_t;

class DLL_MODIFIER CImPdu
{
public:
    CImPdu();
    virtual ~CImPdu() {}
    
    uchar_t* GetBuffer();
    uint32_t GetLength();
    uchar_t* GetBodyData();
    uint32_t GetBodyLength();
    
    uint16_t GetVersion() { return m_pdu_header.version; }
    uint16_t GetFlag() { return m_pdu_header.flag; }
    uint16_t GetServiceId() { return m_pdu_header.service_id; }
    uint16_t GetCommandId() { return m_pdu_header.command_id; }
    uint16_t GetSeqNum() { return m_pdu_header.seq_num; }
    uint32_t GetReversed() { return m_pdu_header.reversed; }
    
    void SetVersion(uint16_t version);
    void SetFlag(uint16_t flag);
    void SetServiceId(uint16_t service_id);
    void SetCommandId(uint16_t command_id);
    void SetError(uint16_t error);
    void SetSeqNum(uint16_t seq_num);
    void SetReversed(uint32_t reversed);
    
    // PDU完整性检查
    static bool IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len);
    // PDU完整解析
    static CImPdu* ReadPdu(uchar_t* buf, uint32_t len);
    // PDU头部写入
    void WriteHeader();
    // PDU头部解析
    int ReadPduHeader(uchar_t* buf, uint32_t len);
    // 缓冲区数据写入
    void Write(uchar_t* buf, uint32_t len) { m_buf.Write((void*)buf, len); }
    // 将Protobuf消息设置为当前消息体
    bool SetPBMsg(const google::protobuf::MessageLite* msg);
    
protected:
    CSimpleBuffer	m_buf;          // 消息
    PduHeader_t		m_pdu_header;   // 消息头
};

#endif // _IMPDUBASE_H_

