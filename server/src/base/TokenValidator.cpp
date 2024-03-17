/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: TokenValidator.cpp
 Update Time: Tue 13 Jun 2023 17:09:37 CST
 brief:
*/

#include "TokenValidator.h"

// 字符串常量，用作令牌生成过程中的密钥
#define AUTH_ENCRYPT_KEY "Mgj!@#123" 

// Constants are the integer part of the sines of integers (in radians) * 2^32.
// 定义 k[64] 和 r[] 用于MD5哈希计算的常量
const uint32_t k[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// r specifies the per-round shift amounts
const uint32_t r[] = { 
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};


/**
 * leftrotate function definition 用于左循环移位操作 
 * 接受两个参数 x 和 c，并返回将 x 向左循环移位 c 位后的结果
 * 
 * 宏定义展开后的表达式为 (((x) << (c)) | ((x) >> (32 - (c))))
 *  - (x) << (c) 表示将 x 向左移动 c 位，空出的低位用零填充
 *  - (x) >> (32 - (c)) 表示将 x 向右移动 32 - c 位，空出的高位用零填充
 *  - | 表示按位或操作，将左移和右移的结果合并起来
 * 这样宏定义 LEFTROTATE(x, c) 实现了将 x 向左循环移位 c 位的操作，保留了移位后的结果
 * 在一些位操作中，左循环移位常用于循环算法和加密算法的实现中
*/
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))


/// @brief 将一个 uint32_t 类型的整数值转换为字节数组
/// @param val 要转换的uint32_t类型的整数值
/// @param bytes 指向uint8_t类型的字节数组的指针
void to_bytes(uint32_t val, uint8_t* bytes) {
    //将val的低字节存储在bytes[0]中
    bytes[0] = (uint8_t)val;
    //次低字节存储在bytes[1]中 依此类推
    bytes[1] = (uint8_t)(val >> 8);
    bytes[2] = (uint8_t)(val >> 16);
    bytes[3] = (uint8_t)(val >> 24);
    //通过右移操作val >> 8、val >> 16 和 val >> 24
    //将val中的高位逐步移动到低位，并将其强制转换为 uint8_t 类型后存储在相应的字节位置上
    //val >> 8 用于将变量val的二进制表示向右移动8位
}

/// @brief 将长度为4字节的字节数组转换为一个 uint32_t 类型的整数
/// @param bytes 指向字节数组的指针
/// @return uint32_t
uint32_t to_int32(const uint8_t* bytes) {
    /**
     * 函数中的每一行代码都将字节数组中的一个字节转换为 uint32_t 类型
     * 并根据其位置进行位移操作
     * 然后将这些结果通过 按位或运算符 | 进行合并
     * 得到最终的 uint32_t 值
    */
    return (uint32_t)bytes[0]
        | ((uint32_t)bytes[1] << 8)
        | ((uint32_t)bytes[2] << 16)
        | ((uint32_t)bytes[3] << 24);
}


/// @brief C实现的MD5哈希算法
/// @param initial_msg 初始消息
/// @param initial_len 消息的长度
/// @param digest 缓冲区用于存储生成的MD5哈希值
void md5(const uint8_t* initial_msg, size_t initial_len, uint8_t* digest) {
    // These vars will contain the hash
    uint32_t h0, h1, h2, h3;

    // Message (to prepare)
    uint8_t* msg = NULL;

    size_t new_len, offset;
    uint32_t w[16];
    uint32_t a, b, c, d, i, f, g, temp;

    // 1.使用预定义的常量初始化哈希变量 h0、h1、h2 和 h3
    // Initialize variables - simple count in nibbles:
    h0 = 0x67452301;
    h1 = 0xefcdab89;
    h2 = 0x98badcfe;
    h3 = 0x10325476;

    // 2.预处理消息
    // Pre-processing:
    // append "1" bit to message
    // append "0" bits until message length in bits ≡ 448 (mod 512)
    // append length mod (2^64) to message
    // 2-1.计算填充位后的消息长度
    for (new_len = initial_len + 1; new_len % (512 / 8) != 448 / 8; new_len++);

    // 2-2.分配内存以存储填充后的消息
    msg = (uint8_t*)malloc(new_len + 8);

    // 2-3.将初始消息复制到填充后的消息
    memcpy(msg, initial_msg, initial_len);

    // 2-4.在消息末尾追加 "1" 比特和零比特，直到长度满足模 512 余 448 的条件
    msg[initial_len] = 0x80; // append the "1" bit; most significant bit is "first"

    // 2-5.将填充后的消息的剩余部分设置为零比特0 这样做是为了确保消息的长度满足模 512 余 448 的条件
    for (offset = initial_len + 1; offset < new_len; offset++) msg[offset] = 0; // append "0" bits

    // 2-6.在消息末尾追加初始消息的比特长度
    // append the len in bits at the end of the buffer.
    to_bytes(initial_len * 8, msg + new_len);
    // initial_len>>29 == initial_len*8>>32, but avoids overflow.
    to_bytes(initial_len >> 29, msg + new_len + 4);

    // 3.照 512 比特块对消息进行处理
    // Process the message in successive 512-bit chunks:
    // for each 512-bit chunk of message:
    for (offset = 0; offset < new_len; offset += (512 / 8)) {
        // 3-1.将每个块分解为十六个 32 比特的单词（w[j]）
        for (i = 0; i < 16; i++) w[i] = to_int32(msg + offset + i * 4);

        // 3-2.使用哈希值初始化临时变量 a、b、c 和 d
        a = h0;
        b = h1;
        c = h2;
        d = h3;

        // 3-3.根据当前迭代进行一系列位操作和变换，更新 a、b、c 和 d
        for (i = 0; i < 64; i++) {
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }
            temp = d;
            d = c;
            c = b;
            b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);//左循环位移进行加密
            a = temp;
        }

        // 4.将当前块的哈希值添加到当前结果中，更新哈希变量 h0、h1、h2 和 h3
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
    }

    // 5.清理用于填充后的消息的内存
    free(msg);

    // 6.使用 to_bytes 函数将生成的哈希值转换为字节，并将其存储在 digest 缓冲区中
    to_bytes(h0, digest);
    to_bytes(h1, digest + 4);
    to_bytes(h2, digest + 8);
    to_bytes(h3, digest + 12);
}


/// @brief 用于生成令牌token
/// 使用 MD5 哈希算法来计算字符串的MD5值，并将结果以字符串的形式存储在 md5_str_buf 中
/// @param uid 用户id
/// @param time_offset 时间偏移量
/// @param md5_str_buf 指向字符数组的指针
/// @return 0 if generate token successful
int genToken(unsigned int uid, time_t time_offset, char* md5_str_buf) {
    // MD5_CTX ctx;
    // 1.定义了一些临时缓冲区和变量，包括 tmp_buf、t_buf 和 md5_buf
    char tmp_buf[256];
    char t_buf[128];
    unsigned char md5_buf[32];

    // 2.通过调用系统相关的函数获取当前时间，并将其格式化为字符串存储在t_buf中
#ifdef _WIN32
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    snprintf(t_buf, sizeof(t_buf), "%04d-%02d-%02d-%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour);
#else
    struct tm* tm;
    time_t currTime;
    time(&currTime);
    currTime += time_offset;
    tm = localtime(&currTime);
    snprintf(t_buf, sizeof(t_buf), "%04d-%02d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour);
#endif

    // 3.使用 snprintf 函数将字符串 tmp_buf 格式化为 %s_%u_%s_%s 的形式
    // AUTH_ENCRYPT_KEY预定义的密钥 uid用户id 混合
    snprintf(tmp_buf, sizeof(tmp_buf), "%s_%u_%s_%s", AUTH_ENCRYPT_KEY, uid, t_buf, AUTH_ENCRYPT_KEY);

    // 4.将 tmp_buf 的内容作为输入进行 MD5 计算，将结果存储在 md5_buf 中
    md5((unsigned char*)tmp_buf, strlen(tmp_buf), md5_buf);

    // 5.使用循环将 md5_buf 的每个字节转换为两位的十六进制字符串，并存储在 md5_str_buf 中
    for (int i = 0; i < 16; i++) sprintf(md5_str_buf + 2 * i, "%02x", md5_buf[i]);

    // 6.反转 md5_str_buf 的内容，将首尾的字符依次交换位置
    char c = 0;
    for (int i = 0; i < 16; i++) {
        c = md5_str_buf[i];
        md5_str_buf[i] = md5_str_buf[31 - i];
        md5_str_buf[31 - i] = c;
    }

    // 7.交换 md5_str_buf 中每两个连续字符的位置
    // switch md5_str_buf[i] and md5_str_buf[i + 1]
    for (int i = 0; i < 32; i += 2) {
        c = md5_str_buf[i];
        md5_str_buf[i] = md5_str_buf[i + 1];
        md5_str_buf[i + 1] = c;
    }
    return 0;
}


/// @brief 验证用户令牌token是否有效
/// @param user_id 用户id
/// @param token 
/// @return 
bool IsTokenValid(uint32_t user_id, const char* token) {
    // 1.定义了三个临时缓冲区用于存储生成的令牌
    char token1[64], token2[64], token3[64];

    // 2.调用 genToken 函数生成了三个令牌，分别对应用户id在过去一小时、当前时间和未来一小时的情况下生成的令牌
    genToken(user_id, -3600, token1); // token an hour ago
    genToken(user_id, 0, token2); // current token
    genToken(user_id, 3600, token3); // token an hour later

    // 3.如果传入的令牌与任何一个生成的令牌相等，则返回true表示令牌有效
    if (!strcmp(token, token1) || !strcmp(token, token2) || !strcmp(token, token3)) return true;
    return false;
}

