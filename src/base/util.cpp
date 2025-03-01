/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.cpp
 Update Time: Mon 12 Jun 2023 17:04:14 CST
 brief: 各种工具类封装
*/

#include "util.h"
#include <sstream>
using namespace std;

// 初始化CSLog对象 用于系统打印日志
CSLog g_imlog = CSLog(LOG_MODULE_IM);

///////////////////////////////////////////////////////////////////////////////////////////
CRefObject::CRefObject() {
    m_lock = NULL;
    m_refCount = 1;
}

CRefObject::~CRefObject() { }

void CRefObject::AddRef() {
    if (m_lock) {
        m_lock->lock();
        m_refCount++;
        m_lock->unlock();
    } else {
        m_refCount++;
    }
}

void CRefObject::ReleaseRef() {
    //首先检查是否有关联的锁对象m_lock
    if (m_lock) {
        //如果有先对锁进行加锁操作 再进行减1操作
        m_lock->lock();
        m_refCount--;
        if (m_refCount == 0) {
            delete this;
            return;
        }
        m_lock->unlock();
    } else {
        //如果没有直接对引用计数进行减1操作
        m_refCount--;
        if (m_refCount == 0) delete this;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 根据不同的操作系统环境获取当前的系统时间或高精度计时器值
 * 在Windows环境下 使用QueryPerformanceCounter和QueryPerformanceFrequency获取高精度计时器的值，
 * 然后计算出以毫秒ms为单位的时间戳，并返回该值
 * 
 * 在非Windows环境下
 * 使用gettimeofday函数获取当前时间
 * 然后将秒数乘以 1000 并加上微秒数除以 1000 的结果，得到以毫秒为单位的时间戳，并返回该值
*/
uint64_t get_tick_count() {
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

/**
 * 用于在不同的操作系统环境下暂停（睡眠）指定的时间
 * 作用是在程序执行过程中暂停一段时间，用于实现延迟、定时等功能
*/
void util_sleep(uint32_t millisecond) {
#ifdef _WIN32
    Sleep(millisecond);
#else
    usleep(millisecond * 1000);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 构造函数
 * 将一个字符串按照指定的分隔符分割成多个子字符串，并存储在动态分配的数组中以便后续使用和访问
*/
CStrExplode::CStrExplode(char* str, char seperator) {
    m_item_cnt = 1;
    char* pos = str;//起始位置

    // 1.首先计算分割后的子字符串数量，通过遍历字符串中的字符来确定
    while (*pos) {
        if (*pos == seperator) m_item_cnt++;
        pos++;
    }

    // 2.动态分配一个大小为m_item_cnt的char*数组，用于存储分割后的子字符串
    m_item_list = new char*[m_item_cnt];

    // 3.再次遍历字符串，将每个子字符串拷贝到对应的数组元素中
    int idx = 0;//子字符串在数组中的位置
    char* start = pos = str;
    while (*pos) {
        if (pos != start && *pos == seperator) {
            // 将子字符串存入数组元素中
            uint32_t len = pos - start;
            m_item_list[idx] = new char[len + 1];
            strncpy(m_item_list[idx], start, len);
            m_item_list[idx][len] = '\0';
            idx++;
            start = pos + 1;
        }
        pos++;
    }

    // 4.如果最后一个子字符串不以分隔符结尾，需要单独处理并将其拷贝到数组中
    uint32_t len = pos - start;
    if (len != 0) {
        m_item_list[idx] = new char[len + 1];
        strncpy(m_item_list[idx], start, len);
        m_item_list[idx][len] = '\0';
    }
}

CStrExplode::~CStrExplode() {
    for (uint32_t i = 0; i < m_item_cnt; i++) delete[] m_item_list[i];
    delete[] m_item_list;
}

///////////////////////////////////////////////////////////////////////////////////////////

/// @brief 替换字符串中的指定的字符
/// @param pSrc 指向源字符串的指针
/// @param oldChar 替换目标字符
/// @param newChar 替换为新字符
/// @return char* 返回源字符串 pSrc
char* replaceStr(char* pSrc, char oldChar, char newChar) {
    if (NULL == pSrc) return NULL;
    char* pHead = pSrc;
    while (*pHead != '\0') {
        if (*pHead == oldChar) *pHead = newChar;
        ++pHead;
    }
    return pSrc;
}

/// @brief 无符号整数转换为字符串表示
/// @param user_id 
/// @return 
string int2string(uint32_t user_id) {
    // 使用 std::stringstream 对象 ss 来实现转换
    // 将无符号整数user_id插入到std::stringstream对象ss中 这种用法会将整数转换为字符串，并将其添加到流中
    stringstream ss;
    ss << user_id;
    return ss.str();
}

uint32_t string2int(const string& value) {
    return (uint32_t)atoi(value.c_str());
}

/// @brief 从pos位置开始 将字符串str中的?替换为字符串'new_value'
/// @param str 字符串
/// @param new_value 替换的字符串
/// @param begin_pos 开始查找find的位置
void replace_mark(string& str, string& new_value, uint32_t& begin_pos) {
    // 由于被替换的内容可能包含?号，所以需要更新开始搜寻的位置信息来避免替换刚刚插入的?号
    // 1.在字符串str中从起始位置begin_pos开始搜索第一个出现的字符 ?
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) return;//如果没有找到 则直接返回不进行任何修改

    // 2.构造一个新字符串 prime_new_value = 'new_value'
    string prime_new_value = "'" + new_value + "'";

    // 3.将字符串str中找到的位置pos处的字符 替换为prime_new_value
    str.replace(pos, 1, prime_new_value);

    // 4.更新begin_pos位置 以便下一次搜索可以从更新后的位置开始
    begin_pos = pos + prime_new_value.size();
}


/// @brief 从pos位置开始 将字符串str中的?替换为字符串'new_value'
/// @param str 字符串
/// @param new_value 替换的数子
/// @param begin_pos 开始查找find的位置
void replace_mark(string& str, uint32_t new_value, uint32_t& begin_pos) {
    // 1.将数字转为字符串
    stringstream ss;
    ss << new_value;
    string str_value = ss.str();

    // 2.在字符串str中从起始位置begin_pos开始搜索第一个出现的字符 ?
    string::size_type pos = str.find('?', begin_pos);
    if (pos == string::npos) return;

    // 3.将字符串str中找到的位置pos处的字符 替换为 str_value
    str.replace(pos, 1, str_value);

    // 4.更新begin_pos位置 以便下一次搜索可以从更新后的位置开始
    begin_pos = pos + str_value.size();
}

//将当前进程的PID写入到名为 server.pid 的文件中
void writePid() {
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t)GetCurrentProcess();
#else
    curPid = (uint32_t)getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    //1.文件打开失败，会触发断言错误
    assert(f);
    //2.使用snprintf函数将当前进程的PID格式化为字符串 并将其存储在szPid中
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    //3.使用fwrite函数将szPid中的字符串内容写入到文件中
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}

/// @brief 将数字转换为对应十六进制字符
/// @param x 要转换的数字（int->char）
/// @return 
inline unsigned char toHex(const unsigned char& x) {
    return x > 9 ? x - 10 + 'A' : x + '0';
}

/// @brief 将十六进制表示的字符转换为对应的数值
/// @param x 要转换的十六进制字符char
/// @return （char->int）
inline unsigned char fromHex(const unsigned char& x) {
    return isdigit(x) ? x - '0' : x - 'A' + 10;
}

/// @brief 将字符串进行URL编码
/// @param sIn 输入的字符串
/// @return 
string URLEncode(const string& sIn) {
    // 1.创建一个空的字符串 sOut，用于存储编码后的结果
    string sOut;

    // 2.遍历输入字符串 sIn 的每个字符，使用循环控制变量 ix
    for (size_t ix = 0; ix < sIn.size(); ix++) {
        // 2-1.对于每个字符，首先创建一个长度为 4 的缓冲区 buf，并将其初始化为 0
        unsigned char buf[4];
        memset(buf, 0, 4);
        // 2-2.如果字符是字母或数字，则直接将其放入缓冲区 buf[0] 中
        if (isalnum((unsigned char)sIn[ix])) buf[0] = sIn[ix];
        else {
            // 2-3.如果字符不是字母或数字，则需要进行 URL 编码
            // 将 % 放入缓冲区 buf[0] 中，然后分别将字符的高 4 位和低 4 位转换为十六进制，并放入缓冲区 buf[1] 和 buf[2] 中
            buf[0] = '%';
            buf[1] = toHex((unsigned char)sIn[ix] >> 4);
            buf[2] = toHex((unsigned char)sIn[ix] % 16);
        }
        // 2-4.将缓冲区 buf 转换为 char* 类型，并将其添加到结果字符串 sOut 中
        sOut += (char*)buf;
    }
    return sOut;
}

/// @brief 对URL编码的字符串进行解码
/// @param sIn 输入的编码字符串
/// @return 
string URLDecode(const string& sIn) {
    // 1.创建一个空的字符串 sOut，用于存储解码后的结果
    string sOut;

    // 2.历输入字符串 sIn 的每个字符，使用循环控制变量 ix
    for (size_t ix = 0; ix < sIn.size(); ix++) {
        unsigned char ch = 0;
        if (sIn[ix] == '%') {
            // 2-1.对于每个字符，如果是 %，则表示需要进行解码操作 
            // 根据 % 后面的两个字符，将其转换为对应的 ASCII 字符，并将其存储在变量 ch 中
            ch = (fromHex(sIn[ix + 1]) << 4);
            ch |= fromHex(sIn[ix + 2]);
            ix += 2;
        } else if (sIn[ix] == '+') {
            // 2-2.如果字符是 +，则表示需要将其解码为空格字符
            ch = ' ';
        } else {
            // 2-3.如果字符既不是 % 也不是 +，则直接将其存储在变量 ch 中
            ch = sIn[ix];
        }
        // 2-4.将变量 ch 转换为 char 类型，并将其添加到结果字符串 sOut 中
        sOut += (char)ch;
    }

    return sOut;
}

//获取文件的大小
int64_t get_file_size(const char* path) {
    int64_t filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0) {
        return filesize;//获取文件信息失败
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
}

/// @brief 在内存中查找子字符串
/// @param src_str 要搜索的源字符串的指针
/// @param src_len 源字符串的长度
/// @param sub_str 要查找的子字符串的指针
/// @param sub_len 子字符串的长度
/// @param flag 搜索的方向 true从左向右 false从右向左
/// @return 
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag) {
    if (NULL == src_str || NULL == sub_str || src_len <= 0) return NULL;
    if (src_len < sub_len) return NULL;

    // 1.根据子字符串长度是否为 0 来确定子字符串的实际长度
    const char* p;
    if (sub_len == 0) sub_len = strlen(sub_str);
    if (sub_len == src_len) {
        if (0 == (memcmp(src_str, sub_str, src_len))) return src_str;
        else return NULL;
    }

    // 2.函数根据搜索方向标志 flag 的值，选择从左向右还是从右向左进行搜索
    if (flag) {
        // 2-1.从左向右搜索
        // 逐个比较源字符串中的子字符串，并检查是否匹配 返回该子字符串在源字符串中的指针位置
        for (int i = 0; i < src_len - sub_len; i++) {
            p = src_str + i;
            if (0 == memcmp(p, sub_str, sub_len)) return p;
        }
    } else {
        // 2-1.从右向左搜索
        for (int i = (src_len - sub_len); i >= 0; i--) {
            p = src_str + i;
            if (0 == memcmp(p, sub_str, sub_len)) return p;
        }
    }

    return NULL;
}

