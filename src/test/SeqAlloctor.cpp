/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: SeqAlloctor.cpp
 Update Time: Thu 15 Jun 2023 01:02:06 CST
 brief:
*/

#include "SeqAlloctor.h"
#include <stdlib.h>

CSeqAlloctor* CSeqAlloctor::m_pInstance = NULL;

CSeqAlloctor::CSeqAlloctor()
{
}

CSeqAlloctor::~CSeqAlloctor()
{
    if (m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

CSeqAlloctor* CSeqAlloctor::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CSeqAlloctor();
    }
    return m_pInstance;
}

uint32_t CSeqAlloctor::getSeq(uint32_t nType)
{
    auto it = m_hmAlloctor.find(nType);
    uint32_t nSeqNo = 0;
    if (it != m_hmAlloctor.end()) {
        it->second++;
        nSeqNo = it->second;
    } else {
        srand(time(NULL));
        nSeqNo = rand() + 1;
        m_hmAlloctor[nType] = nSeqNo;
    }
    return nSeqNo;
}