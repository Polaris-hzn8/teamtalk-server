/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImPduBase.h
 Update Time: Mon 12 Jun 2023 23:48:57 CST
 brief: 用于通信数据包的读取与解析
*/

#ifndef IMPDUBASE_H_
#define IMPDUBASE_H_

#include "UtilPdu.h"
#include "pb/google/protobuf/message_lite.h"

#define IM_PDU_HEADER_LEN		16
#define IM_PDU_VERSION			1

//名为 ALLOC_FAIL_ASSERT 的宏定义 用于检查指针p是否为NULL，如果是则抛出一个类型为CPduException的异常
#define ALLOC_FAIL_ASSERT(p) if (p == NULL) { \
throw CPduException(m_pdu_header.service_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed"); \
}

//CHECK_PB_PARSE_MSG 宏定义用于检查Protobuf消息解析的结果 如果解析结果为false
#define CHECK_PB_PARSE_MSG(ret) { \
    if (ret == false) \
    {\
        log_error("parse pb msg failed.");\
        return;\
    }\
}

/**
 * DLL_MODIFIER 是一个宏定义，在不同的操作系统和编译环境下具有不同的定义
 * DLL_MODIFIER 被用来修饰导出和导入 DLL（动态链接库）的函数和数据
 * 
 *  __declspec(dllexport)：在 Windows 平台上，__declspec(dllexport) 用于标记函数和数据的导出，表示它们将在动态链接库中作为公共接口可用
 *  __declspec(dllimport)：在 Windows 平台上，__declspec(dllimport) 用于标记函数和数据的导入，表示它们将从动态链接库中导入使用
 *  当不在 Windows 操作系统下进行编译时 定义 DLL_MODIFIER 为空，即不进行任何修饰
*/
#ifdef WIN32

#ifdef BUILD_PDU
#define DLL_MODIFIER __declspec(dllexport)
#else
#define DLL_MODIFIER __declspec(dllimport)
#endif

#else

#define DLL_MODIFIER

#endif

/////////////////////////////PduHeader_t通信协议头部结构///////////////////////////////////
typedef struct {
    uint32_t 	length;//整个PDU的长度
    uint16_t 	version;//PDU版本号
    uint16_t	flag;//未使用
    uint16_t	service_id;//服务id
    uint16_t	command_id;//命令id
    uint16_t	seq_num;//包序号
    uint16_t    reversed;//预留字段
} PduHeader_t;

class DLL_MODIFIER CImPdu {
public:
    CImPdu();
    virtual ~CImPdu() {}
    
    //返回内部缓冲区 m_buf 的指针
    uchar_t* GetBuffer();
    //返回 PDU 的总长度，即 PDU 头部和消息体的总长度
    uint32_t GetLength();
    //返回 PDU 消息体的数据指针
    uchar_t* GetBodyData();
    //返回 PDU 消息体的长度
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

    //将 m_pdu_header 中的数据写入到内部的缓冲区 m_buf 中
    void WriteHeader();
    
    //检查给定的缓冲区中是否存在完整的 PDU 数据
    static bool IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len);
    //从给定的缓冲区中解析出一个完整的PDU，并返回一个新创建的CImPdu对象
    static CImPdu* ReadPdu(uchar_t* buf, uint32_t len);
    //将给定的数据写入到内部的缓冲区 m_buf 中
    void Write(uchar_t* buf, uint32_t len) { m_buf.Write((void*)buf, len); }
    //从给定的缓冲区中解析出 PDU 头部，并更新 m_pdu_header 成员的值
    int ReadPduHeader(uchar_t* buf, uint32_t len);
    //将给定的 Protobuf 消息设置为当前 PDU 对象的消息体
    void SetPBMsg(const google::protobuf::MessageLite* msg);
    
protected:
    //内部缓冲区对象 用于存储PDU的数据（消息头+消息体）
    CSimpleBuffer	m_buf;
    //PDU头部结构体 存储PDU的头部信息
    PduHeader_t		m_pdu_header;
};

#endif



