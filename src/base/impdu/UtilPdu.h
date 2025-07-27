/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UtilPdu.h
 Update Time: Mon 12 Jun 2023 09:03:36 CST
 brief: pdu工具模块，实现一些数据结构（字节数组等）来处理字节型的数据、字节流数据处理的一些工具类
*/

#ifndef UTILPDU_H_
#define UTILPDU_H_

#include <set>
#include <map>
#include <list>
#include <string>
#include "common/ostype.h"

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

/**
 * CPduException 类用于处理 PDU 过程中的异常情况 以便捕获和处理错误情况
 * 通过获取异常对象的属性，可以获取有关错误的详细信息，如服务ID、命令ID、错误代码和错误消息，从而进行相应的错误处理
*/
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
 * CSimpleBuffer 类提供一个简单的缓冲区实现，用于在内存中存储数据
 * 可以通过扩展缓冲区的大小来适应不断增长的数据量，并提供了写入和读取数据的方法，方便进行数据的存储和读取操作
 * 用于网络通信或数据序列化等场景，提供了基本的缓冲区功能。
*/
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

/**
 * CByteStream类提供了一些方便的方法来读取和写入不同类型的数据（例如整数、字符串、字节数组）到缓冲区中 以及从缓冲区中读取数据
 * 这些方法可以帮助简化数据的序列化和反序列化过程。
*/
class CByteStream {
public:
	CByteStream(uchar_t* buf, uint32_t len);
	CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos);
	~CByteStream() {}

	//获取string类型字符串形式 类似c_str()
	//返回内部缓冲区的指针
	unsigned char* GetBuf() { return m_pSimpBuf ? m_pSimpBuf->GetBuffer() : m_pBuf; }
	//返回当前读写位置
	uint32_t GetPos() { return m_pos; }
	//返回缓冲区的长度
	uint32_t GetLen() { return m_len; }

	//跳过指定长度的数据，将读写位置向后移动
	void Skip(uint32_t len) {
		m_pos += len;
		if(m_pos > m_len) throw CPduException(ERROR_CODE_PARSE_FAILED, "parase packet failed!");
	}

	//从给定缓冲区中读取 16 位有符号整数并返回
	static int16_t ReadInt16(uchar_t* buf);
	//从给定缓冲区中读取 32 位有符号整数并返回
	static int32_t ReadInt32(uchar_t* buf);
	//将 16 位有符号整数写入到给定缓冲区中
	static void WriteInt16(uchar_t* buf, int16_t data);
	//将 32 位有符号整数写入到给定缓冲区中
	static void WriteInt32(uchar_t* buf, int32_t data);

	//从给定缓冲区中读取 16 位无符号整数并返回
	static uint16_t ReadUint16(uchar_t* buf);
	//从给定缓冲区中读取 32 位无符号整数并返回
	static uint32_t ReadUint32(uchar_t* buf);
	//将 16 位无符号整数写入到给定缓冲区中
	static void WriteUint16(uchar_t* buf, uint16_t data);
	//将 32 位无符号整数写入到给定缓冲区中
	static void WriteUint32(uchar_t* buf, uint32_t data);

	//重载左移运算符 将一个 8 | 16 | 32 位有符号整数写入到缓冲区中
	void operator << (int8_t data);
	void operator << (int16_t data);
	void operator << (int32_t data);
	void operator << (uint8_t data);
	void operator << (uint16_t data);
	void operator << (uint32_t data);

	//重载右移运算符，从缓冲区中读取一个 8 | 16 | 32 位有符号整数并赋值给指定变量
	void operator >> (int8_t& data);
	void operator >> (int16_t& data);
	void operator >> (int32_t& data);
	void operator >> (uint8_t& data);
	void operator >> (uint16_t& data);
	void operator >> (uint32_t& data);
	
	//将字符串写入到缓冲区中
	void WriteString(const char* str);
	//将指定长度的字符串写入到缓冲区中
	void WriteString(const char* str, uint32_t len);
	//从缓冲区中读取字符串并返回 同时将字符串的长度保存到指定变量中
	char* ReadString(uint32_t& len);

	//将给定数据写入到缓冲区中
	void WriteData(uchar_t* data, uint32_t len);
	//从缓冲区中读取数据并返回，同时将数据的长度保存到指定变量中
	uchar_t* ReadData(uint32_t& len);
private:
	void _WriteByte(void* buf, uint32_t len);
	void _ReadByte(void* buf, uint32_t len);
private:
	CSimpleBuffer*	m_pSimpBuf;//指向 CSimpleBuffer 对象的指针，用于处理数据读写操作
	uchar_t*		m_pBuf;//指向缓冲区的指针
	uint32_t		m_len;//缓冲区的长度
	uint32_t		m_pos;//当前读写位置
};

//将一个uint32_t类型的整数转换为对应的URL字符串
char* idtourl(uint32_t id);

//将URL字符串转换为对应的uint32_t类型的整数
uint32_t urltoid(const char* url);

#endif
