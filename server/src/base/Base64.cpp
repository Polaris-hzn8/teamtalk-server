/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Base64.cpp
 Update Time: Sun 11 Jun 2023 16:29:45 CST
 brief: 
*/

#include <stdio.h>
#include <iostream>
#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

using namespace std;

// base64编码表 用于将6位二进制值映射到相应的Base64字符
static const char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// reverse_table解码表 用于将Base64字符映射回原始的6位二进制值
static const char reverse_table[128] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

/**
 * @brief base64编码过程
 * 
 * @param const string &bindata 待编码的01二进制数据
 * @return string Base64编码字符串
*/
string base64_encode(const string &bindata) {
    using std::numeric_limits;
    
    // 1.首先检查待编码的数据是否过大
    // numeric_limits<string::size_type>::max()获取 string::size_type 类型的最大值
    // 将最大值除以4 再乘以3 以确定进行Base64编码后的最大字符串长度
    if (bindata.size() > (numeric_limits<string::size_type>::max() / 4u) * 3u) {
        //throw length_error("Converting too large a string to base64.");
        /* 如果待编码的数据长度超过限制 返回空字符串 */
        return "";
    }

    // 2.根据二进制数据的长度 计算编码后的字符串长度
    const size_t binlen = bindata.size();
    // 初始化string对象来存储编码结果: 第一个参数为字符串长度 第二个参数为初始化字符串的值
    // 使用(binlen + 2) / 3可以确保编码后的字符串长度能够容纳所有的数据
    // 乘以4是因为每3个字节的数据会编码为4个字符（3 * 8 = 4 * 6）
    string retval((((binlen + 2) / 3) * 4), '=');


    // 3.函数通过迭代遍历输入数据的每个字节，并将字节转换为对应的Base64字符
    size_t outpos = 0;//retval中存储字符的位置
    int bits_collected = 0;//跟踪累加器中已经收集的比特位数
    unsigned int accumulator = 0;//用于存储累加器的值
    const string::const_iterator binend = bindata.end();//指向字符串bindata结尾位置的迭代器

    for (string::const_iterator i = bindata.begin(); i != binend; ++i) {
        //每次迭代将当前字节与0xffu按位与运算
        //将其左移8位并与accumulator进行按位或运算 以确保只取字节的低8位
        accumulator = (accumulator << 8) | (*i & 0xffu);
        //将新的字节添加到累加器中
        bits_collected += 8;
        //当累加器中的比特位数bits_collected达到6位或以上时
        while (bits_collected >= 6) {
            bits_collected -= 6;//表示已从累加器中取出了6位值
            //将accumulator右移并与0x3fu按位与运算 以获取累加器中的6位值对应的索引
            int index = (accumulator >> bits_collected) & 0x3fu;
            //根据索引从b64_table中找到相应的Base64字符，并将其存入retval中
            retval[outpos++] = b64_table[index];
        }
    }

    // 4.最后如果存在剩余的不足6位的二进制位 则将累加器左移使剩余位移动到高位 再次索引并存入retval
    // Any trailing bits that are missing.
    // 利用断言assert验证条件是否为真
    if (bits_collected > 0) {
        //剩余不足6位的二进制位 但bits_collected值大于等于6 断言失败并抛出异常（值错误或算法逻辑错误）
        assert(bits_collected < 6);
        //剩余的不足6位的二进制位 且bits_collected < 6 则进行左移操作
        accumulator <<= 6 - bits_collected;
        //并通过与0x3fu按位与运算获取索引
        int index =  accumulator & 0x3fu;
        //根据索引从b64_table中找到相应的Base64字符，并将其存入retval中
        retval[outpos++] = b64_table[index];
    }

    // 5.通过不断更新的outpos来确定retval中存储字符的位置 使用断言assert进行一些边界检查 确保编码结果的正确性
    // 确保outpos的值不小于retval的长度减去2：是否正确计算了编码结果的长度，并且没有发生越界访问
    assert(outpos >= (retval.size() - 2));
    // 确保outpos的值不大于等于retval的长度：检查是否正确更新了outpos的值，避免越界写入retval
    assert(outpos <= retval.size());

    return retval;   
}


/**
 * @brief base64解码过程
 * 对Base64编码字符串进行逐字符解码 并将解码后的二进制数据以字符串形式存储在retval中
 * 
 * @param const string &ascdata Base64编码字符串
 * @return string 01二进制数据
*/
string base64_decode(const string &ascdata) {
    string retval;
    const string::const_iterator last = ascdata.end();//ascdata字符串的末尾位置的迭代器
    int bits_collected = 0;//表示当前已收集的二进制位数
    unsigned int accumulator = 0;//用于累积收集的二进制数据的变量

    // 逐字符解码 并将解码后的二进制数据以字符串形式存储在retval中    
    for (string::const_iterator i = ascdata.begin(); i != last; ++i) {
        const int c = *i;
        // 1.当前字符是否为空格或者为等号，如果是则跳过该字符
        // 在Base64编码中空格和填充字符'='是可以存在的，但对解码结果没有影响 故直接跳过
        if (isspace(c) || c == '=') continue;

        // 2.检查当前字符是否在合法的范围内，异常返回空字符串
        if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) return "";

        // 3.将收集到的二进制数据累积到accumulator中
        // 将accumulator左移6位 并与reverse_table[c]按位或运算
        accumulator = (accumulator << 6) | reverse_table[c];
        // 将bits_collected增加6，表示已经收集了6位二进制数据
        bits_collected += 6;

        // 4.如果bits_collected达到或超过8，表示已经收集足够的二进制数据可以解码为一个字节
        if (bits_collected >= 8) {
            bits_collected -= 8;
            //从accumulator中取出最高位的8位二进制数据，并将其转换为字符类型添加到retval字符串中
            retval += (char)((accumulator >> bits_collected) & 0xffu);
        }
    }
    return retval;
}

