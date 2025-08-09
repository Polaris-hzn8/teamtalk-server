/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DepartModel.h
 Update Time: Thu 15 Jun 2023 00:29:27 CST
 brief:
*/

#ifndef __DEPARTMODEL_H__
#define __DEPARTMODEL_H__

#include <list>
#include "ImPduBase.h"
#include "IM.BaseDefine.pb.h"

class CDepartModel
{
public:
    ~CDepartModel() {}
    static CDepartModel* getInstance();
    void getChgedDeptId(uint32_t& nLastTime, std::list<uint32_t>& lsChangedIds);
    void getDepts(std::list<uint32_t>& lsDeptIds, std::list<IM::BaseDefine::DepartInfo>& lsDepts);
    void getDept(uint32_t nDeptId, IM::BaseDefine::DepartInfo& cDept);

private:
    CDepartModel() {};
private:
    static CDepartModel* m_pInstance;
};

#endif /*defined(__DEPARTMODEL_H__) */
