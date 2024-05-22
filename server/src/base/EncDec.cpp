/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: EncDec.cpp
 Update Time: Mon 12 Jun 2023 08:30:45 CST
 brief: 
*/

#include "EncDec.h"
#include "Base64.h"
#include "UtilPdu.h"
#include <stdio.h>
#include <string.h>

/// @brief 在构造函数中使用AES算法设置 加密和解密密钥
/// @param strKey 明文密码
CAes::CAes(const string& strKey) {
    AES_set_encrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cEncKey);
    AES_set_decrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cDecKey);
}

/// @brief 对输入数据进行AES加密
/// 在使用该函数之前，需要确保相关的AES加密库和头文件已正确引入，并链接到程序中
/// @param pInData 指向要加密的输入数据的指针
/// @param nInLen 输入数据的长度
/// @param ppOutData 指向指针的指针，用于存储加密后的数据
/// @param nOutLen 加密后的数据长度
/// @return int
int CAes::Encrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen) {
    if (pInData == NULL || nInLen <= 0) return -1;

    // 1.计算加密后数据的长度
    uint32_t nRemain = nInLen % 16;
    uint32_t nBlocks = (nInLen + 15) / 16;

    // 2.并根据加密算法的要求进行数据块大小的调整
    if (nRemain > 12 || nRemain == 0) nBlocks += 1;
    uint32_t nEncryptLen = nBlocks * 16;

    // 3.动态分配内存用于存储加密前和加密后的数据
    unsigned char* pData = (unsigned char*)calloc(nEncryptLen, 1);//加密前的数据
    memcpy(pData, pInData, nInLen);
    unsigned char* pEncData = (unsigned char*)malloc(nEncryptLen);//加密后的数据

    // 4.将原始数据的长度（nInLen）写入到最后一个数据块的末尾
    CByteStream::WriteUint32((pData + nEncryptLen - 4), nInLen);

    // 5.使用AES_encrypt函数对每个数据块进行AES加密，将加密后的结果存储在pEncData中
    for (uint32_t i = 0; i < nBlocks; i++) AES_encrypt(pData + i * 16, pEncData + i * 16, &m_cEncKey);

    // 6.完成加密后释放原始数据的内存 并将加密后的数据转换为string类型
    free(pData);
    string strEnc((char*)pEncData, nEncryptLen);
    free(pEncData);

    // 7.使用base64_encode函数对加密后的数据进行Base64编码，将编码后的结果存储在strDec中
    string strDec = base64_encode(strEnc);

    // 8.确定编码后数据的长度 并为其分配内存
    nOutLen = (uint32_t)strDec.length();
    char* pTmp = (char*)malloc(nOutLen + 1);//指向存储加密后并经过Base64编码的数据的字符指针

    // 9.将编码后的数据复制到新分配的内存中，并在末尾添加字符串结束符
    memcpy(pTmp, strDec.c_str(), nOutLen);
    pTmp[nOutLen] = 0;

    // 10.将指向加密后数据的指针赋值给ppOutData，以便在函数外部可以访问到加密后的数据
    *ppOutData = pTmp;

    return 0;
}

/// @brief 解密函数Decrypt的实现
/// @param pInData 指向待解密数据的字符数组的指针
/// @param nInLen 待解密数据的长度 以字节为单位
/// @param ppOutData 指向char*类型指针的指针，用于存储解密后的数据
/// @param nOutLen uint32_t类型的引用，用于存储解密后的数据的长度
/// @return int
int CAes::Decrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen) {
    if (pInData == NULL || nInLen <= 0) return -1;

    // 1.将输入数据pInData和长度nInLen构造成string对象strInData
    string strInData(pInData, nInLen);

    // 2.使用base64_decode函数对strInData进行Base64解码
    std::string strResult = base64_decode(strInData);

    // 3.获取解码后数据的长度nLen并进行判断 如果解码后的数据长度为0表示解码失败返回-2
    uint32_t nLen = (uint32_t)strResult.length();
    if (nLen == 0) return -2;

    // 4.将解码后的数据转换为const unsigned char*类型 并赋值给pData
    const unsigned char* pData = (const unsigned char*)strResult.c_str();

    // 5.检查待解密数据的长度是否是16的倍数如果不是，即存在不完整的AES加密块 返回-3
    // 如果待解密数据的长度不是16的倍数，说明数据不完整或格式错误，无法正确解密
    if (nLen % 16 != 0) return -3;
    
    // 6.根据解码后的数据长度nLen为 pTmp 分配足够的内存空间用于存储解密后的数据（解密完成后的长度应该小于该长度）
    char* pTmp = (char*)malloc(nLen + 1);

    // 7.通过循环遍历将解码后的数据进行分块AES解密
    uint32_t nBlocks = nLen / 16;
    for (uint32_t i = 0; i < nBlocks; i++) AES_decrypt(pData + i * 16, (unsigned char*)pTmp + i * 16, &m_cDecKey);

    // 8.在解密后的数据末尾，使用CByteStream::ReadUint32函数从解密后数据的末尾读取出长度值，并将其赋值给 nOutLen
    uchar_t* pStart = (uchar_t*)pTmp + nLen - 4;
    nOutLen = CByteStream::ReadUint32(pStart);
    
    // 9.判断解密后的数据长度nOutLen是否大于解密前的数据长度nLen
    // 如果是表示解密后的数据长度异常 释放pTmp所指向的内存空间并返回-4
    if (nOutLen > nLen) {
        free(pTmp);
        return -4;
    }

    // 10.将解密后的数据以空字符结尾 并将pTmp赋值给*ppOutData，将解密后的数据指针输出
    pTmp[nOutLen] = 0;//将其最后一个字符置为null终止符 \0 以确保字符串的正确结束
    *ppOutData = pTmp;
    return 0;
}

/// @brief 安全地释放动态分配的内存 确保不会导致内存泄漏或使用已释放的内存
/// @param pOutData 指向需要释放内存的空间的指针
void CAes::Free(char* pOutData) {
    if (pOutData) {
        free(pOutData);
        pOutData = NULL;
    }
}

/// @brief 计算给定数据的MD5哈希值
/// @param pContent 指向要计算哈希值的数据的指针
/// @param nLen 数据的长度
/// @param md5 存储计算得到的MD5哈希值的字符串缓冲区
void CMd5::MD5_Calculate(const char* pContent, unsigned int nLen, char* md5) {
    uchar_t d[16];//存储计算得到的MD5哈希值
    MD5_CTX ctx;//MD5_CTX结构体类型的变量ctx 存储MD5计算的上下文信息
    MD5_Init(&ctx);//初始化MD5计算的上下文
    MD5_Update(&ctx, pContent, nLen);//利用MD5_Update函数将数据块pContent的内容添加到MD5计算中
    MD5_Final(d, &ctx);//使用MD5_Final函数将最终的MD5哈希值存储到数组d中

    //循环通过将每个字节转换为两位的十六进制表示，并使用snprintf函数将结果存储在字符串缓冲区md5中
    for (int i = 0; i < 16; ++i) snprintf(md5 + (i * 2), 32, "%02x", d[i]);
    //将字符串缓冲区的最后一个字符设置为零，以表示字符串的结尾。
    md5[32] = 0;
    return;
}
