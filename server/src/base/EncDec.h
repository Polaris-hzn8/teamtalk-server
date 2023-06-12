/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: EncDec.h
 Update Time: Mon 12 Jun 2023 08:31:18 CST
 brief: 
*/

#ifndef __ENCDEC_H__
#define __ENCDEC_H__

#include <iostream>
#include <openssl/aes.h>//大量引用
#include <openssl/md5.h>//大量引用
#include "ostype.h"

using namespace std;

class CAes {
public:
    CAes(const string& strKey);
    int Encrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen);
    int Decrypt(const char* pInData, uint32_t nInLen, char** ppOutData, uint32_t& nOutLen);
    void Free(char* pData);
private:
    AES_KEY m_cEncKey;
    AES_KEY m_cDecKey;
};

class CMd5 {
public:
    static void MD5_Calculate (const char* pContent, unsigned int nLen,char* md5);
};

#endif