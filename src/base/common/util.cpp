/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.cpp
 Update Time: Mon 12 Jun 2023 17:04:14 CST
 brief: 各种工具类封装
*/

#include "util.h"
#include <sstream>

CSLog g_imlog = CSLog(LOG_MODULE_IM);

///////////////////////////////////////////////////////////////////////////////////////////
// CRefObject
CRefObject::CRefObject()
{
    m_lock = NULL;
    m_refCount = 1;
}

CRefObject::~CRefObject()
{

}

void CRefObject::AddRef()
{
    if (m_lock) {
        m_lock->lock();
        m_refCount++;
        m_lock->unlock();
    } else {
        m_refCount++;
    }
}

void CRefObject::ReleaseRef()
{
    if (m_lock) {
        m_lock->lock();
        m_refCount--;
        if (m_refCount == 0) {
            delete this;
            return;
        }
        m_lock->unlock();
    } else {
        m_refCount--;
        if (m_refCount == 0)
            delete this;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// CStrExplode
CStrExplode::CStrExplode(char* str, char seperator)
{
    m_item_cnt = 1;
    char* pos = str;

    while (*pos) {
        if (*pos == seperator) m_item_cnt++;
        pos++;
    }

    m_item_list = new char*[m_item_cnt];

    int idx = 0;
    char* start = pos = str;
    while (*pos) {
        if (pos != start && *pos == seperator) {
            uint32_t len = pos - start;
            m_item_list[idx] = new char[len + 1];
            strncpy(m_item_list[idx], start, len);
            m_item_list[idx][len] = '\0';
            idx++;
            start = pos + 1;
        }
        pos++;
    }

    uint32_t len = pos - start;
    if (len != 0) {
        m_item_list[idx] = new char[len + 1];
        strncpy(m_item_list[idx], start, len);
        m_item_list[idx][len] = '\0';
    }
}

CStrExplode::~CStrExplode()
{
    for (uint32_t i = 0; i < m_item_cnt; i++)
        delete[] m_item_list[i];
    delete[] m_item_list;
}

///////////////////////////////////////////////////////////////////////////////////////////
std::string int2string(uint32_t user_id)
{
    std::stringstream ss;
    ss << user_id;
    return ss.str();
}

uint32_t string2int(const std::string& value)
{
    return (uint32_t)atoi(value.c_str());
}

uint64_t get_tick_count()
{
#ifdef _WIN32
    LARGE_INTEGER liCounter;
    LARGE_INTEGER liCurrent;
    if (!QueryPerformanceFrequency(&liCounter)) return GetTickCount();
    QueryPerformanceCounter(&liCurrent);
    return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
    struct timeval tval;
    uint64_t ret_tick;
    gettimeofday(&tval, NULL);
    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond)
{
#ifdef _WIN32
    Sleep(millisecond);
#else
    usleep(millisecond * 1000);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
// 替换字符串中的指定字符
char* replaceStr(char* pSrc, char oldChar, char newChar)
{
    if (NULL == pSrc) return NULL;
    char* pHead = pSrc;
    while (*pHead != '\0') {
        if (*pHead == oldChar) *pHead = newChar;
        ++pHead;
    }
    return pSrc;
}

void replace_mark(std::string& str, std::string& new_value, uint32_t& begin_pos)
{
    std::string::size_type pos = str.find('?', begin_pos);
    if (pos == std::string::npos)
        return;

    std::string prime_new_value = "'" + new_value + "'";

    str.replace(pos, 1, prime_new_value);

    begin_pos = pos + prime_new_value.size();
}

void replace_mark(std::string& str, uint32_t new_value, uint32_t& begin_pos)
{
    std::stringstream ss;
    ss << new_value;
    std::string str_value = ss.str();

    std::string::size_type pos = str.find('?', begin_pos);
    if (pos == std::string::npos)
        return;

    str.replace(pos, 1, str_value);

    begin_pos = pos + str_value.size();
}

void writePid()
{
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t)GetCurrentProcess();
#else
    curPid = (uint32_t)getpid();
#endif
    FILE* fp = fopen("server.pid", "w");
    assert(fp);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, fp);
    fclose(fp);
}

inline unsigned char toHex(const unsigned char& x)
{
    return x > 9 ? x - 10 + 'A' : x + '0';
}

inline unsigned char fromHex(const unsigned char& x)
{
    return isdigit(x) ? x - '0' : x - 'A' + 10;
}

std::string URLEncode(const std::string& sIn)
{
    std::string sOut;
    for (size_t ix = 0; ix < sIn.size(); ix++) {
        unsigned char buf[4];
        memset(buf, 0, 4);
        if (isalnum((unsigned char)sIn[ix])) {
            buf[0] = sIn[ix];
        } else {
            buf[0] = '%';
            buf[1] = toHex((unsigned char)sIn[ix] >> 4);
            buf[2] = toHex((unsigned char)sIn[ix] % 16);
        }
        sOut += (char*)buf;
    }
    return sOut;
}

std::string URLDecode(const std::string& sIn)
{
    std::string sOut;
    for (size_t ix = 0; ix < sIn.size(); ix++) {
        unsigned char ch = 0;
        if (sIn[ix] == '%') {
            ch = (fromHex(sIn[ix + 1]) << 4);
            ch |= fromHex(sIn[ix + 2]);
            ix += 2;
        } else if (sIn[ix] == '+') {
            ch = ' ';
        } else {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
    return sOut;
}

int64_t get_file_size(const char* path)
{
    int64_t filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0) {
        return filesize;//获取文件信息失败
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
}

// 内存中查找字符串
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag)
{
    if (NULL == src_str || NULL == sub_str || src_len <= 0)
        return NULL;
    if (src_len < sub_len)
        return NULL;

    const char* p;
    if (sub_len == 0)
        sub_len = strlen(sub_str);
    if (sub_len == src_len) {
        if (0 == (memcmp(src_str, sub_str, src_len))) return src_str;
        else return NULL;
    }

    if (flag) {
        /* 从左向右搜索 */
        for (int i = 0; i < src_len - sub_len; i++) {
            p = src_str + i;
            if (0 == memcmp(p, sub_str, sub_len))
                return p;
        }
    } else {
        /* 从右向左搜索 */
        for (int i = (src_len - sub_len); i >= 0; i--) {
            p = src_str + i;
            if (0 == memcmp(p, sub_str, sub_len))
                return p;
        }
    }

    return NULL;
}

