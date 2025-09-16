/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DepartModel.cpp
 Update Time: Thu 15 Jun 2023 00:29:19 CST
 brief:
*/

#include "DBPool.h"
#include "DepartModel.h"
using namespace std;

CDepartModel* CDepartModel::m_pInstance = NULL;

CDepartModel* CDepartModel::getInstance()
{
    if (NULL == m_pInstance) {
        m_pInstance = new CDepartModel();
    }
    return m_pInstance;
}

//从数据库中获取最近更新时间后发生变化的部门ID列表
void CDepartModel::getChgedDeptId(uint32_t& nLastTime, list<uint32_t>& lsChangedIds)
{
    // 1.获取 CDBManager 的单例实例
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");

    if (pDBConn) {
        // 2.如果成功获取到数据库连接对象
        // 2-1.构建SQL查询语句，查询IMDepart表中更新时间大于给定最后更新时间 nLastTime 的部门ID和更新时间
        string strSql = "select id, updated from IMDepart where updated > " + int2string(nLastTime);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());//结果集对象 pResultSet
        if (pResultSet) {
            //遍历结果集，对于每条记录获取部门ID和更新时间
            while (pResultSet->Next()) {
                uint32_t id = pResultSet->GetInt("id");//部门id
                uint32_t nUpdated = pResultSet->GetInt("updated");//更新时间
                //如果部门的更新时间大于当前最后更新时间 nLastTime，则更新 nLastTime 的值为部门的更新时间
                if (nLastTime < nUpdated) nLastTime = nUpdated;
                //将部门ID添加到变更的部门ID列表 lsChangedIds 中
                lsChangedIds.push_back(id);
            }
            delete pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    } else {
        // 3.如果未能获取到数据库连接对象 记录错误日志
        log("no db connection for teamtalk_slave.");
    }
}

//从数据库中获取给定部门ID列表中的部门信息
void CDepartModel::getDepts(list<uint32_t>& lsDeptIds, list<IM::BaseDefine::DepartInfo>& lsDepts)
{
    if (lsDeptIds.empty()) {
        log("list is empty");
        return;
    }

    // 1.获取 CDBManager 的单例实例
    CDBManager* pDBManager = CDBManager::getInstance();
    // 通过pDBManager获取一个指向teamtalk_slave数据库连接的CDBConn对象
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");

    if (pDBConn) {
        // 2.成功获取到数据库连接对象
        // 2-1.构建SQL查询语句，使用给定的部门ID列表构建查询条件
        string strClause;
        bool bFirst = true;
        for (auto it = lsDeptIds.begin(); it != lsDeptIds.end(); ++it) {
            if (bFirst) {
                bFirst = false;
                strClause += int2string(*it);
            } else {
                strClause += ("," + int2string(*it));
            }
        }
        string strSql = "select * from IMDepart where id in ( " + strClause + " )";

        // 2-2.执行SQL查询，获取结果集对象 pResultSet
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet) {
            // 2-3.遍历结果集对于每条记录，获取部门的ID 父部门ID 部门名称 部门状态和优先级
            while (pResultSet->Next()) {
                IM::BaseDefine::DepartInfo cDept;
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nParentId = pResultSet->GetInt("parentId");
                string strDeptName = pResultSet->GetString("departName");
                uint32_t nStatus = pResultSet->GetInt("status");
                uint32_t nPriority = pResultSet->GetInt("priority");
                // 2-4.如果当前部门状态是有效的，创建一个 IM::BaseDefine::DepartInfo 对象，并设置其属性
                if (IM::BaseDefine::DepartmentStatusType_IsValid(nStatus)) {
                    cDept.set_dept_id(nId);
                    cDept.set_parent_dept_id(nParentId);
                    cDept.set_dept_name(strDeptName);
                    cDept.set_dept_status(IM::BaseDefine::DepartmentStatusType(nStatus));
                    cDept.set_priority(nPriority);
                    // 2-5.将部门信息对象添加到部门信息列表lsDepts中
                    lsDepts.push_back(cDept);
                }
            }
            delete pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    } else {
        // 3.未能获取到数据库连接对象 则记录日志
        log("no db connection for teamtalk_slave");
    }
}

void CDepartModel::getDept(uint32_t nDeptId, IM::BaseDefine::DepartInfo& cDept)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("teamtalk_slave");
    if (pDBConn) {
        string strSql = "select * from IMDepart where id = " + int2string(nDeptId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet) {
            while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("id");
                uint32_t nParentId = pResultSet->GetInt("parentId");
                string strDeptName = pResultSet->GetString("departName");
                uint32_t nStatus = pResultSet->GetInt("status");
                uint32_t nPriority = pResultSet->GetInt("priority");
                if (IM::BaseDefine::DepartmentStatusType_IsValid(nStatus)) {
                    cDept.set_dept_id(nId);
                    cDept.set_parent_dept_id(nParentId);
                    cDept.set_dept_name(strDeptName);
                    cDept.set_dept_status(IM::BaseDefine::DepartmentStatusType(nStatus));
                    cDept.set_priority(nPriority);
                }
            }
            delete pResultSet;
        }
        pDBManager->RelDBConn(pDBConn);
    } else {
        log("no db connection for teamtalk_slave");
    }
}