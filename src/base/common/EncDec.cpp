/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: EncDec.cpp
 Update Time: Mon 12 Jun 2023 08:30:45 CST
 brief: 
*/

#include "EncDec.h"
#include "Base64.h"
#include "impdu/UtilPdu.h"
#include <stdio.h>
#include <string.h>

// 加解密秘钥设置
CAes::CAes(const std::string& strKey)
{
    AES_set_encrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cEncKey);
    AES_set_decrypt_key((const unsigned char*)strKey.c_str(), 256, &m_cDecKey);
}

int CAes::Encrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
{
    if (pInData == NULL || nInLen <= 0)
        return -1;

    uint32_t nRemain = nInLen % 16;
    uint32_t nBlocks = (nInLen + 15) / 16;

    if (nRemain > 12 || nRemain == 0)
        nBlocks += 1;
    uint32_t nEncryptLen = nBlocks * 16;

    // 加密前的数据
    unsigned char* pData = (unsigned char*)calloc(nEncryptLen, 1);
    memcpy(pData, pInData, nInLen);

    // 加密后的数据
    unsigned char* pEncData = (unsigned char*)malloc(nEncryptLen);
    CByteStream::WriteUint32((pData + nEncryptLen - 4), nInLen);
    for (uint32_t i = 0; i < nBlocks; i++)
        AES_encrypt(pData + i * 16, pEncData + i * 16, &m_cEncKey);

    free(pData);
    std::string strEnc((char*)pEncData, nEncryptLen);
    free(pEncData);

    // base64编码
    std::string strDec = base64_encode(strEnc);

    nOutLen = (uint32_t)strDec.length();
    char* pTmp = (char*)malloc(nOutLen + 1);
    memcpy(pTmp, strDec.c_str(), nOutLen);
    pTmp[nOutLen] = 0;

    *ppOutData = pTmp;

    return 0;
}

int CAes::Decrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen)
{
    if (pInData == NULL || nInLen <= 0)
        return -1;

    std::string strInData(pInData, nInLen);

    // base64解码
    std::string strRet = base64_decode(strInData);
    uint32_t nLen = (uint32_t)strRet.length();
    if (nLen == 0)
        return -2;

    const unsigned char* pData = (const unsigned char*)strRet.c_str();

    // 待解密数据长度不是16倍数 数据不完整或格式错误 存在不完整的AES加密块
    if (nLen % 16 != 0)
        return -3;
    
    char* pTmp = (char*)malloc(nLen + 1);

    // 将数据进行分块AES解密
    uint32_t nBlocks = nLen / 16;
    for (uint32_t i = 0; i < nBlocks; i++)
        AES_decrypt(pData + i * 16, (unsigned char*)pTmp + i * 16, &m_cDecKey);

    // 解密后的数据长度不可大于原始长度
    uchar_t* pStart = (uchar_t*)pTmp + nLen - 4;
    nOutLen = CByteStream::ReadUint32(pStart);
    if (nOutLen > nLen) {
        free(pTmp);
        return -4;
    }

    pTmp[nOutLen] = 0;
    *ppOutData = pTmp;
    return 0;
}

void CAes::Free(char* pOutData)
{
    if (pOutData) {
        free(pOutData);
        pOutData = NULL;
    }
}

void CMd5::MD5_Calculate(const char* pContent, unsigned int nLen, char* md5)
{
    if (!pContent || !md5)
        return;

    uchar_t dist[16];
    MD5_CTX ctx;
    MD5_Init(&ctx);                     // 初始化上下文
    MD5_Update(&ctx, pContent, nLen);   // 提供数据
    MD5_Final(dist, &ctx);              // 最终结果

    for (int i = 0; i < 16; ++i)
        sprintf(md5 + (i * 2), "%02x", dist[i]);

    md5[32] = '\0';
    return;
}
