/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UtilPdu.h
 Update Time: Mon 12 Jun 2023 09:03:36 CST
 brief: pdu工具模块，实现一些数据结构（字节数组等）来处理字节型的数据、字节流数据处理的一些工具类
*/

#ifndef UTILPDU_H_
#define UTILPDU_H_

#include "ostype.h"
#include <set>
#include <map>
#include <list>
#include <string>

using namespace std;

/*
	1.关于条件编译以及宏定义DLL_MODIFIER的解释：
	#ifdef WIN32判断是否为win32平台
		如果是则：
			#ifdef BUILD_PDU
				如果定义 则DLL_MODIFIER被定义为 __declspec(dllexport)
				否则 DLL_MODIFIER被定义为 __declspec(dllimport)
		如果不是则：DLL_MODIFIER被定义为空，不做任何修饰
	通过这样的条件编译，可以在Windows平台上使用DLL_MODIFIER来控制函数或变量的导入和导出，以确保模块之间的正确链接和调用

	2.关于__declspec(dllexport)的解释
		是Microsoft Visual C++编译器提供的一个扩展，用于指定将符号（函数、变量等）导出到动态链接库DLL中
		当在一个DLL中定义一个函数或变量时，如果希望其他程序或模块能够访问这些符号，就需要使用 __declspec(dllexport) 关键字进行标记
		这样编译器在编译DLL时会将这些标记的符号导出，使得其他模块可以通过动态链接库进行链接和调用
		
		在编译 DLL 时将符号导出，在使用 DLL 的应用程序中将符号导入。这种方式用于确保在DLL和应用程序之间正确地进行符号的共享和访问
		该方式是特定于 Microsoft Visual C++ 编译器的扩展 在其他编译器上可能具有不同的语法或机制来控制符号的导出
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

// exception code
#define ERROR_CODE_PARSE_FAILED 		1
#define ERROR_CODE_WRONG_SERVICE_ID		2
#define ERROR_CODE_WRONG_COMMAND_ID		3
#define ERROR_CODE_ALLOC_FAILED			4

class CPduException {
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
	string		m_error_msg;
};

class DLL_MODIFIER CSimpleBuffer {
public:
	CSimpleBuffer();
	~CSimpleBuffer();
	uchar_t*  GetBuffer() { return m_buffer; }
	uint32_t GetAllocSize() { return m_alloc_size; }
	uint32_t GetWriteOffset() { return m_write_offset; }
	void IncWriteOffset(uint32_t len) { m_write_offset += len; }

	//扩展内部缓冲区的大小
	void Extend(uint32_t len);
	//向内部缓冲区写入数据
	uint32_t Write(void* buf, uint32_t len);
	//从内部缓冲区读取数据
	uint32_t Read(void* buf, uint32_t len);
private:
	uchar_t*	m_buffer;//内部缓冲区的指针uchar_t*
	uint32_t	m_alloc_size;//内部缓冲区的总大小
	uint32_t	m_write_offset;//当前写入位置的偏移量 
};

class CByteStream {
public:
	CByteStream(uchar_t* buf, uint32_t len);
	CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos);
	~CByteStream() {}

	//获取string类型字符串形式 类似c_str()
	unsigned char* GetBuf() { return m_pSimpBuf ? m_pSimpBuf->GetBuffer() : m_pBuf; }
	uint32_t GetPos() { return m_pos; }
	uint32_t GetLen() { return m_len; }

	void Skip(uint32_t len) {
		m_pos += len;
		if(m_pos > m_len) throw CPduException(ERROR_CODE_PARSE_FAILED, "parase packet failed!");
	}

	static int16_t ReadInt16(uchar_t* buf);
	static int32_t ReadInt32(uchar_t* buf);
	static void WriteInt16(uchar_t* buf, int16_t data);
	static void WriteInt32(uchar_t* buf, int32_t data);

	static uint16_t ReadUint16(uchar_t* buf);
	static uint32_t ReadUint32(uchar_t* buf);
	static void WriteUint16(uchar_t* buf, uint16_t data);
	static void WriteUint32(uchar_t* buf, uint32_t data);

	void operator << (int8_t data);
	void operator << (int16_t data);
	void operator << (int32_t data);
	void operator << (uint8_t data);
	void operator << (uint16_t data);
	void operator << (uint32_t data);

	void operator >> (int8_t& data);
	void operator >> (int16_t& data);
	void operator >> (int32_t& data);
	void operator >> (uint8_t& data);
	void operator >> (uint16_t& data);
	void operator >> (uint32_t& data);
	
	void WriteString(const char* str);
	void WriteString(const char* str, uint32_t len);
	char* ReadString(uint32_t& len);

	void WriteData(uchar_t* data, uint32_t len);
	uchar_t* ReadData(uint32_t& len);
private:
	void _WriteByte(void* buf, uint32_t len);
	void _ReadByte(void* buf, uint32_t len);
private:
	CSimpleBuffer*	m_pSimpBuf;
	uchar_t*		m_pBuf;//unsigned char
	uint32_t		m_len;
	uint32_t		m_pos;
};

char* idtourl(uint32_t id);
uint32_t urltoid(const char* url);

#endif
