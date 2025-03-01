/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: RelationModel.h
 Update Time: Thu 15 Jun 2023 00:32:37 CST
 brief:
*/

#ifndef RELATION_SHIP_H_
#define RELATION_SHIP_H_

#include <list>

#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"
#include "util.h"

using namespace std;

class CRelationModel {
public:
    virtual ~CRelationModel();

    static CRelationModel* getInstance();
    uint32_t getRelationId(uint32_t nUserAId, uint32_t nUserBId, bool bAdd);
    bool updateRelation(uint32_t nRelationId, uint32_t nUpdateTime);
    bool removeRelation(uint32_t nRelationId);

private:
    CRelationModel();
    uint32_t addRelation(uint32_t nSmallId, uint32_t nBigId);

private:
    static CRelationModel* m_pInstance;
};
#endif
