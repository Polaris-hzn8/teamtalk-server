/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UtilPdu.cpp
 Update Time: Mon 12 Jun 2023 09:44:35 CST
 brief:
    为什么要对流数据进行操作？
    1.本质上是在处理网络协议、文件格式、进程通信中的数据序列化问题
    2.使用CByteStream将结构体写入到流中
    3.然后通过send将流式数据发送出去
*/

#include <stdlib.h>
#include <string.h>
#include "UtilPdu.h"

///////////// CSimpleBuffer ////////////////
CSimpleBuffer::CSimpleBuffer()
{
    m_buffer = NULL;
    m_alloc_size = 0;
    m_write_offset = 0;
}

CSimpleBuffer::~CSimpleBuffer()
{
    m_alloc_size = 0;
    m_write_offset = 0;
    if (m_buffer) {
        free(m_buffer);
        m_buffer = NULL;
    }
}

bool CSimpleBuffer::Extend(uint32_t len)
{
    if (m_write_offset + len < m_write_offset)
        return false;//溢出

    // new_size
    uint32_t new_size = m_write_offset + len;
    new_size += new_size >> 2;

    // new_buf
    uchar_t* new_buf = (uchar_t*)realloc(m_buffer, new_size);
    if (!new_buf)
        return false;
        
    m_buffer = new_buf;
    m_alloc_size = new_size;
    return true;
}

uint32_t CSimpleBuffer::Write(void* buf, uint32_t len)
{
    if (!buf || len <= 0)
        return 0;
    
    if (len > UINT32_MAX - m_write_offset)
        return 0;

    if (m_write_offset + len > m_alloc_size)
        if (!Extend(len))
            return 0;

    // data write
    memcpy(m_buffer + m_write_offset, buf, len);
    m_write_offset += len;
    return len;
}

uint32_t CSimpleBuffer::Read(void* buf, uint32_t len)
{
    if (!buf || !m_buffer || len == 0 || m_write_offset == 0)
        return 0;

    // data read
    len = std::min(len, m_write_offset);
    memcpy(buf, m_buffer, len);

    m_write_offset -= len;
    if (m_write_offset)
        memmove(m_buffer, m_buffer + len, m_write_offset);//向前移动len字节(有剩余数据)

    return len;
}

/////////////////// CByteStream ///////////////////
CByteStream::CByteStream(uchar_t* buf, uint32_t len)
{
    m_pBuf = buf;
    m_len = len;
    m_pSimpBuf = NULL;
    m_pos = 0;
}

CByteStream::CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos)
{
    m_pSimpBuf = pSimpBuf;
    m_pos = pos;
    m_pBuf = NULL;
    m_len = 0;
}

bool CByteStream::Skip(uint32_t len)
{
    if (len > UINT32_MAX - m_pos)
        return false;
    m_pos += len;
    if(m_pos > m_len) {
        throw CPduException(ERROR_CODE_PARSE_FAILED, "skip exceeds buffer length.");
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////
/*
    CByteStream - group1
    1.用于将数据转换为字节流格式，以便在网络传输或存储中使用
    2.通过位运算和类型转换 将不同大小的整数类型数据 与字节数组进行相互转换
        (uint16_t int6_t uint32_t int32_t)
*/
int16_t CByteStream::ReadInt16(uchar_t* buf)
{
    int16_t data = (buf[0] << 8) | buf[1];
    return data;
}

int32_t CByteStream::ReadInt32(uchar_t* buf)
{
    int32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

uint16_t CByteStream::ReadUint16(uchar_t* buf)
{
    uint16_t data = (buf[0] << 8) | buf[1];
    return data;
}

uint32_t CByteStream::ReadUint32(uchar_t* buf)
{
    /**
     * 通过位运算将缓冲区中的4个字节按照大端字节序（Big-Endian）进行合并
     * 构造出一个32位整数数据
     * 1.buf[0] << 24 将 buf 缓冲区的第一个字节左移24位
     * 2.buf[1] << 16 将 buf 缓冲区的第二个字节左移16位
     * 3.buf[2] << 8 将 buf 缓冲区的第三个字节左移8位
     * 4.buf[3] 取 buf 缓冲区的第四个字节
     * 5.进行 或（OR）操作，将四个字节合并为一个32位整数数据 并将结果存储在变量 data 中
    */
    uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return data;
}

void CByteStream::WriteInt16(uchar_t* buf, int16_t data)
{
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteInt32(uchar_t* buf, int32_t data)
{
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint16(uchar_t* buf, uint16_t data)
{
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint32(uchar_t* buf, uint32_t data)
{
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
}

///////////////////////////////////////////////////////////////////////////////
/*
    CByteStream - group2
    运算符重载函数 用于方便地进行数据的写入和读取操作
    1.operator<< 重载运算符用于数据写入
        对于不同的数据类型
        将其转换为字节数组,
        并通过_WriteByte将字节数组写入到字节流中
    2.operator>> 重载运算符用于数据读取
        对于不同的数据类型
        从字节流中读取相应字节数的数据_ReadByte
        并将其转换为对应的数据类型
    3.这些运算符重载函数
        使得在使用CByteStream类时
        可以像使用流操作符一样方便地进行数据的写入和读取
        提供了一种简洁的方式来处理字节流数据
*/
void CByteStream::operator<<(int8_t data)
{
    _WriteByte(&data, 1);
}

void CByteStream::operator<<(uint8_t data)
{
    _WriteByte(&data, 1);
}

void CByteStream::operator<<(int16_t data)
{
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(uint16_t data)
{
    unsigned char buf[2];
    buf[0] = static_cast<uchar_t>(data >> 8);
    buf[1] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 2);
}

void CByteStream::operator<<(int32_t data)
{
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator<<(uint32_t data)
{
    unsigned char buf[4];
    buf[0] = static_cast<uchar_t>(data >> 24);
    buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
    buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
    buf[3] = static_cast<uchar_t>(data & 0xFF);
    _WriteByte(buf, 4);
}

void CByteStream::operator>>(int8_t& data)
{
    _ReadByte(&data, 1);
}

void CByteStream::operator>>(uint8_t& data)
{
    _ReadByte(&data, 1);
}

void CByteStream::operator>>(int16_t& data)
{
    unsigned char buf[2];
    _ReadByte(buf, 2);
    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(uint16_t& data)
{
    unsigned char buf[2];
    _ReadByte(buf, 2);
    data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator>>(int32_t& data)
{
    unsigned char buf[4];
    _ReadByte(buf, 4);
    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::operator>>(uint32_t& data)
{
    unsigned char buf[4];
    _ReadByte(buf, 4);
    data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

///////////////////////////////////////////////////////////////////////////////////////////
/*
    CByteStream - group3
    字符串与字节流相互转换
    先写入数据长度再写入数据内容
    任意数据写入读出字节流，以便后续的读取或传输操作
*/
// 字符串写入流中
void CByteStream::WriteString(const char* str)
{
    uint32_t size = str ? (uint32_t)strlen(str) : 0;
    *this << size;
    _WriteByte((void*)str, size);
}

void CByteStream::WriteString(const char* str, uint32_t len)
{
    *this << len;
    _WriteByte((void*)str, len);
}

// 从流中读取字符串
char* CByteStream::ReadString(uint32_t& len)
{
    *this >> len;
    char* pStr = (char*)GetBuf() + GetPos();
    Skip(len);
    return pStr;
}

// 写入二进制数据
void CByteStream::WriteData(uchar_t* data, uint32_t len)
{
    *this << len;
    _WriteByte(data, len);
}

// 读取二进制数据
uchar_t* CByteStream::ReadData(uint32_t& len)
{
    *this >> len;
    uchar_t* pData = (uchar_t*)GetBuf() + GetPos();
    Skip(len);
    return pData;
}

// 将字符串写入到字节流中
void CByteStream::_ReadByte(void* buf, uint32_t len)
{
    if (m_pos + len > m_len)
        throw CPduException(ERROR_CODE_PARSE_FAILED, "parase packet failed!");
    if (m_pSimpBuf) {
        m_pSimpBuf->Read((char*)buf, len);
    } else {
        memcpy(buf, m_pBuf + m_pos, len);
    }
    m_pos += len;
}

// 将字节流中的数据读取为字符串
void CByteStream::_WriteByte(void* buf, uint32_t len)
{
    if (m_pBuf && (m_pos + len > m_len))
        return;
    if (m_pSimpBuf) {
        m_pSimpBuf->Write((char*)buf, len);
    } else {
        memcpy(m_pBuf + m_pos, buf, len);
    }
    m_pos += len;
}

///////////////////////////////////////////////////////////////////////////////////
/*
 * Warning!!!
 * This function return a static char pointer, caller must immediately copy the string to other place
 */
char* idtourl(uint32_t id)
{
    static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static char buf[64];
    char* ptr;
    uint32_t value = id * 2 + 56;
    // convert to 36 number system
    ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    do {
        *--ptr = digits[value % 36];
        value /= 36;
    } while (ptr > buf && value);

    // add version number
    *--ptr = '1';

    return ptr;
}

uint32_t urltoid(const char* url)
{
    uint32_t url_len = strlen(url);
    char c;
    uint32_t number = 0;
    for (uint32_t i = 1; i < url_len; i++) {
        // skip version number
        c = url[i];
        if (c >= '0' && c <= '9') {
            c -= '0';
        } else if (c >= 'a' && c <= 'z') {
            c -= 'a' - 10;
        } else if (c >= 'A' && c <= 'Z') {
            c -= 'A' - 10;
        } else {
            continue;
        }
        number = number * 36 + c;
    }
    return (number - 56) >> 1;
}


