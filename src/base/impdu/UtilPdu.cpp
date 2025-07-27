/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UtilPdu.cpp
 Update Time: Mon 12 Jun 2023 09:44:35 CST
 brief: pdu工具模块，实现一些数据结构（字节数组等）来处理字节型的数据、字节流数据处理的一些工具类
*/

#include <stdlib.h>
#include <string.h>
#include "UtilPdu.h"

///////////// CSimpleBuffer ////////////////
CSimpleBuffer::CSimpleBuffer() {
    m_buffer = NULL;
    m_alloc_size = 0;
    m_write_offset = 0;
}

CSimpleBuffer::~CSimpleBuffer() {
    m_alloc_size = 0;
    m_write_offset = 0;
    if (m_buffer) {
        free(m_buffer);
        m_buffer = NULL;
    }
}

void CSimpleBuffer::Extend(uint32_t len) {
    m_alloc_size = m_write_offset + len;
    m_alloc_size += m_alloc_size >> 2;//扩展为原始大小的 1.25(5/4) 倍数
    uchar_t* new_buf = (uchar_t*)realloc(m_buffer, m_alloc_size);
    m_buffer = new_buf;
}

uint32_t CSimpleBuffer::Write(void* buf, uint32_t len) {
    if (m_write_offset + len > m_alloc_size) Extend(len);
    if (buf) memcpy(m_buffer + m_write_offset, buf, len);
    m_write_offset += len;
    return len;
}

uint32_t CSimpleBuffer::Read(void* buf, uint32_t len) {
    if (0 == len) return len;
    if (len > m_write_offset) len = m_write_offset;
    if (buf) memcpy(buf, m_buffer, len);
    m_write_offset -= len;
    //将m_buffer中的数据向前移动len个字节盖原来的数据
    memmove(m_buffer, m_buffer + len, m_write_offset);
    return len;
}

/////////////////// CByteStream ///////////////////
CByteStream::CByteStream(uchar_t* buf, uint32_t len) {
    m_pBuf = buf;
    m_len = len;
    m_pSimpBuf = NULL;
    m_pos = 0;
}

//接受一个指向缓冲区的指针和缓冲区的长度作为参数
CByteStream::CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos) {
    m_pSimpBuf = pSimpBuf;
    m_pos = pos;
    m_pBuf = NULL;
    m_len = 0;
}

///////////////////////////////////////////////////////////////////
//CByteStream - group1
//字节数组和uint16_t int6_t uint32_t int32_t等数字的相互转换
//这些函数用于将数据转换为字节流格式，以便在网络传输或存储中使用。
//通过位运算和类型转换，可以将不同大小的整数类型数据与字节数组进行相互转换

int16_t CByteStream::ReadInt16(uchar_t* buf) {
    int16_t data = (buf[0] << 8) | buf[1];
    return data;
}

uint16_t CByteStream::ReadUint16(uchar_t* buf) {
    uint16_t data = (buf[0] << 8) | buf[1];
    return data;
}

int32_t CByteStream::ReadInt32(uchar_t* buf) {
    int32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

/**
 * 从给定的 buf 缓冲区中读取无符号32位整数（uint32_t）数据
*/
uint32_t CByteStream::ReadUint32(uchar_t* buf) {
    /**
     * 首先通过位运算将缓冲区中的4个字节按照大端字节序（Big-Endian）进行合并 
     * 构造出一个32位整数数据
     * buf[0] << 24 将 buf 缓冲区的第一个字节左移24位
     * buf[1] << 16 将 buf 缓冲区的第二个字节左移16位
     * buf[2] << 8 将 buf 缓冲区的第三个字节左移8位
     * buf[3] 取 buf 缓冲区的第四个字节
     * 进行 或（OR）操作，将四个字节合并为一个32位整数数据 并将结果存储在变量 data 中
    */
    uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

void CByteStream::WriteInt16(uchar_t* buf, int16_t data) {
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint16(uchar_t* buf, uint16_t data) {
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteInt32(uchar_t* buf, int32_t data) {
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint32(uchar_t* buf, uint32_t data) {
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

///////////////////////////////////////////////////////////////////////////////
// CByteStream - group2
// 运算符重载函数 用于方便地进行数据的写入和读取操作
// operator<< 运算符重载函数用于数据的写入操作 对于不同的数据类型，将其转换为字节数组，并通过 _WriteByte 函数将字节数组写入到字节流中
// operator>> 运算符重载函数用于数据的读取操作 对于不同的数据类型，从字节流中读取相应字节数的数据_ReadByte，并将其转换为对应的数据类型
// 这些运算符重载函数使得在使用CByteStream类时，可以像使用流操作符一样方便地进行数据的写入和读取，提供了一种简洁的方式来处理字节流数据

void CByteStream::operator<<(int8_t data) {
    _WriteByte(&data, 1);
}

void CByteStream::operator<<(uint8_t data) {
    _WriteByte(&data, 1);
}

void CByteStream::operator<<(int16_t data) {
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(uint16_t data) {
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(int32_t data) {
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator<<(uint32_t data) {
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator>>(int8_t& data) {
    _ReadByte(&data, 1);
}

void CByteStream::operator>>(uint8_t& data) {
    _ReadByte(&data, 1);
}

void CByteStream::operator>>(int16_t& data) {
    unsigned char buf[2];
    _ReadByte(buf, 2);
    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(uint16_t& data) {
    unsigned char buf[2];
    _ReadByte(buf, 2);
    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(int32_t& data) {
    unsigned char buf[4];
    _ReadByte(buf, 4);
    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::operator>>(uint32_t& data) {
    unsigned char buf[4];
    _ReadByte(buf, 4);
    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::_ReadByte(void* buf, uint32_t len) {
    if (m_pos + len > m_len) throw CPduException(ERROR_CODE_PARSE_FAILED, "parase packet failed!");
    if (m_pSimpBuf) m_pSimpBuf->Read((char*)buf, len);
    else memcpy(buf, m_pBuf + m_pos, len);
    m_pos += len;
}

void CByteStream::_WriteByte(void* buf, uint32_t len) {
    if (m_pBuf && (m_pos + len > m_len)) return;
    if (m_pSimpBuf) m_pSimpBuf->Write((char*)buf, len);
    else memcpy(m_pBuf + m_pos, buf, len);
    m_pos += len;
}

///////////////////////////////////////////////////////////////////////////////////////////
// CByteStream - group3
// 字符串与字节流相互转换
// 任意数据写入读出字节流，以便后续的读取或传输操作

//将字符串写入到字节流中
void CByteStream::WriteString(const char* str) {
    uint32_t size = str ? (uint32_t)strlen(str) : 0;//获取字符串的长度
    *this << size;//使用运算符重载函数operator<<将字符串的长度写入到字节流中（将长度作为一个无符号整数写入）
    _WriteByte((void*)str, size);//将字符串内容写入到字节流中 传入参数为字符串的指针和长度
}

void CByteStream::WriteString(const char* str, uint32_t len) {
    *this << len;
    _WriteByte((void*)str, len);
}

//从字节流中读取字符串
char* CByteStream::ReadString(uint32_t& len) {
    *this >> len;//使用运算符重载函数 operator>> 从字节流中读取字符串的长度 存储在参数len中
    char* pStr = (char*)GetBuf() + GetPos();//获取字节流的缓冲区指针，加上当前的读取位置GetPos()得到字符串的起始地址 pStr
    Skip(len);//跳过已经读取的字符串部分 以便后续的读取操作
    return pStr;
}

//向字节流中写入数据
void CByteStream::WriteData(uchar_t* data, uint32_t len) {
    *this << len;//用运算符重载函数 operator<< 将数据的长度 len 写入字节流
    _WriteByte(data, len);//将数据 data 写入字节流，指定写入的长度为len
}

//从字节流中读取数据
uchar_t* CByteStream::ReadData(uint32_t& len) {
    *this >> len;//使用运算符重载函数 operator>> 从字节流中读取数据的长度 len
    uchar_t* pData = (uchar_t*)GetBuf() + GetPos();//当前读取位置 GetPos() + 字节流的起始地址 GetBuf() = 数据指针pData
    Skip(len);//跳过已读取的数据 以便后续的读取操作
    return pData;
}

///////////////////////////////////////////////////////////////////////////////////

/*
 * Warning!!!
 * This function return a static char pointer, caller must immediately copy the string to other place
 */
// 将一个uint32_t类型的整数转换为对应的URL字符串
char* idtourl(uint32_t id) {
    // 1.用于转换的数字和字母字符
    static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static char buf[64];
    char* ptr;
    uint32_t value = id * 2 + 56;
    // convert to 36 number system
    ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    // 2.循环直到 value 为 0 或者缓冲区指针 ptr 到达缓冲区的起始位置
    do {
        *--ptr = digits[value % 36];
        value /= 36;
    } while (ptr > buf && value);

    // 3.在转换结束后，在缓冲区的最前面添加版本号字符1
    *--ptr = '1'; // add version number

    // 4.返回指向缓冲区的指针ptr
    return ptr;
}

//将URL字符串转换为对应的uint32_t类型的整数
uint32_t urltoid(const char* url) {
    uint32_t url_len = strlen(url);
    char c;
    uint32_t number = 0;//用于存储转换结果的中间值
    for (uint32_t i = 1; i < url_len; i++) {
        //从URL字符串的第二个字符开始遍历（跳过版本号字符1）直到字符串末尾
        c = url[i];
        if (c >= '0' && c <= '9') c -= '0';
        else if (c >= 'a' && c <= 'z') c -= 'a' - 10;
        else if (c >= 'A' && c <= 'Z') c -= 'A' - 10;
        else continue;
        number = number * 36 + c;
    }
    return (number - 56) >> 1;
}


