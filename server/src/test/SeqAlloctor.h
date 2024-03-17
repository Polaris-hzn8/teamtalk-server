/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: SeqAlloctor.h
 Update Time: Thu 15 Jun 2023 01:02:14 CST
 brief:
*/

#ifndef __SEQALLOCTOR_H__
#define __SEQALLOCTOR_H__

#include "ostype.h"

typedef enum {
    ALLOCTOR_PACKET = 1,
} ALLOCTOR_TYPE;

class CSeqAlloctor {
public:
    static CSeqAlloctor* getInstance();
    uint32_t getSeq(uint32_t nType);

private:
    CSeqAlloctor();
    virtual ~CSeqAlloctor();

private:
    static CSeqAlloctor* m_pInstance;
    hash_map<uint32_t, uint32_t> m_hmAlloctor;
};

#endif /*defined(__SEQALLOCTOR_H__) */
