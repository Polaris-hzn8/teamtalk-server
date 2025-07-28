/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UtilPdu.h
 Update Time: Mon 12 Jun 2023 09:03:36 CST
 brief:
 	1.对流式数据进行序列化与反序列化
	2.让原本结构化的数据(字符串、整数、结构体)可以按字节流进行传输、存储或解析
*/

#ifndef UTILPDU_H_
#define UTILPDU_H_

#include <set>
#include <map>
#include <list>
#include <string>
#include "common/ostype.h"

#ifdef WIN32
	#ifdef BUILD_PDU
		#define DLL_MODIFIER __declspec(dllexport)
	#else
		#define DLL_MODIFIER __declspec(dllimport)
	#endif
#else
	#define DLL_MODIFIER
#endif

// exception code
#define ERROR_CODE_PARSE_FAILED 		1
#define ERROR_CODE_WRONG_SERVICE_ID		2
#define ERROR_CODE_WRONG_COMMAND_ID		3
#define ERROR_CODE_ALLOC_FAILED			4
#define ERROR_CODE_OTHERS				5

class CPduException
{
public:
	CPduException(uint32_t service_id, uint32_t command_id, uint32_t error_code, const char* error_msg) {
		m_service_id = service_id;
		m_command_id = command_id;
		m_error_code = error_code;
		m_error_msg = error_msg;
	}
	CPduException(uint32_t error_code, const char* error_msg) {
		m_service_id = 0;
		m_command_id = 0;
		m_error_code = error_code;
		m_error_msg = error_msg;
	}
	virtual ~CPduException() {}

	uint32_t GetServiceId() { return m_service_id; }
	uint32_t GetCommandId() { return m_command_id; }
	uint32_t GetErrorCode() { return m_error_code; }
	char* GetErrorMsg() { return (char*)m_error_msg.c_str(); }
private:
	uint32_t	m_service_id;
	uint32_t	m_command_id;
	uint32_t	m_error_code;
	std::string	m_error_msg;
};

/**
 * @brief 简单的字节缓冲区
 * 1.通过扩展缓冲区大小来适应不断增长的数据量
 * 2.用于网络通信或数据序列化等场景
 */
class DLL_MODIFIER CSimpleBuffer
{
public:
	CSimpleBuffer();
	~CSimpleBuffer();
	
	uchar_t*  	GetBuffer() { return m_buffer; }
	uint32_t 	GetAllocSize() { return m_alloc_size; }
	uint32_t 	GetWriteOffset() { return m_write_offset; }
	void 		IncWriteOffset(uint32_t len) { m_write_offset += len; }

	bool 		Extend(uint32_t len);
	uint32_t 	Write(void* buf, uint32_t len);
	uint32_t 	Read(void* buf, uint32_t len);
private:
	uchar_t*	m_buffer;			//缓冲区指针
	uint32_t	m_alloc_size;		//缓冲区大小
	uint32_t	m_write_offset;		//写入偏移量 
};

// 封装了二进制读写的流式操作类
class CByteStream
{
public:
	CByteStream(uchar_t* buf, uint32_t len);
	CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos);
	~CByteStream() {}

	// 获取缓冲区
	unsigned char* GetBuf() { return m_pSimpBuf ? m_pSimpBuf->GetBuffer() : m_pBuf; }
	// 当前读写位置
	uint32_t GetPos() { return m_pos; }
	// 缓冲区的长度
	uint32_t GetLen() { return m_len; }
	// 读写位置后移
	bool Skip(uint32_t len);

	// 字节数组转换
	static int16_t ReadInt16(uchar_t* buf);
	static int32_t ReadInt32(uchar_t* buf);
	static uint16_t ReadUint16(uchar_t* buf);
	static uint32_t ReadUint32(uchar_t* buf);

	static void WriteInt16(uchar_t* buf, int16_t data);
	static void WriteInt32(uchar_t* buf, int32_t data);	
	static void WriteUint16(uchar_t* buf, uint16_t data);
	static void WriteUint32(uchar_t* buf, uint32_t data);

	// 缓冲区数据写入
	void operator << (int8_t data);
	void operator << (int16_t data);
	void operator << (int32_t data);
	void operator << (uint8_t data);
	void operator << (uint16_t data);
	void operator << (uint32_t data);

	// 缓冲区数据读取
	void operator >> (int8_t& data);
	void operator >> (int16_t& data);
	void operator >> (int32_t& data);
	void operator >> (uint8_t& data);
	void operator >> (uint16_t& data);
	void operator >> (uint32_t& data);
	
	// char*
	void WriteString(const char* str);
	void WriteString(const char* str, uint32_t len);
	char* ReadString(uint32_t& len);

	// unsigned char
	void WriteData(uchar_t* data, uint32_t len);
	uchar_t* ReadData(uint32_t& len);
private:
	void _WriteByte(void* buf, uint32_t len);
	void _ReadByte(void* buf, uint32_t len);
private:
	CSimpleBuffer*	m_pSimpBuf;		//CSimpleBuffer指针
	uchar_t*		m_pBuf;			//缓冲区的指针
	uint32_t		m_len;			//缓冲区长度
	uint32_t		m_pos;			//读写位置
};

char* idtourl(uint32_t id);
uint32_t urltoid(const char* url);

#endif
