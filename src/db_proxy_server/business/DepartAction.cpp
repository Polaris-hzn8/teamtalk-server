/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DepartAction.cpp
 Update Time: Thu 15 Jun 2023 00:28:55 CST
 brief:
*/

#include "ProxyConn.h"
#include "DepartModel.h"
#include "DepartAction.h"
#include "IM.Buddy.pb.h"

namespace DB_PROXY {

/**
 * @brief 部门变更信息获取
 * 
 * @param pPdu          Packet字节流包
 * @param conn_uuid     Socket文件描述符
 */
void getChgedDepart(CImPdu* pPdu, uint32_t conn_uuid)
{
    IM::Buddy::IMDepartmentReq msg;
    IM::Buddy::IMDepartmentRsp msgResp;
    if (msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        CImPdu* pPduRes = new CImPdu;
        uint32_t nUserId = msg.user_id();
        uint32_t nLastUpdate = msg.latest_update_time();

        // 发生变化的部门id
        std::list<uint32_t> lsChangedIds;
        CDepartModel::getInstance()->getChgedDeptId(nLastUpdate, lsChangedIds);
        // 发生变化的部门信息
        std::list<IM::BaseDefine::DepartInfo> lsDeparts;
        CDepartModel::getInstance()->getDepts(lsChangedIds, lsDeparts);

        // 设置部门信息响应消息的用户ID和最新更新时间
        msgResp.set_user_id(nUserId);
        msgResp.set_latest_update_time(nLastUpdate);

        // 遍历部门信息列表，将每个部门信息添加到部门信息响应消息中
        for (auto it = lsDeparts.begin(); it != lsDeparts.end(); ++it) {
            IM::BaseDefine::DepartInfo* pDeptInfo = msgResp.add_dept_list();
            pDeptInfo->set_dept_id(it->dept_id());
            pDeptInfo->set_priority(it->priority());
            pDeptInfo->set_dept_name(it->dept_name());
            pDeptInfo->set_parent_dept_id(it->parent_dept_id());
            pDeptInfo->set_dept_status(it->dept_status());
        }
        log("userId=%u, last_update=%u, cnt=%u", nUserId, nLastUpdate, lsDeparts.size());

        msgResp.set_attach_data(msg.attach_data());             //设置附加消息
        pPduRes->SetPBMsg(&msgResp);                            //设置消息体
        pPduRes->SetSeqNum(pPdu->GetSeqNum());                  //设置消息序号
        pPduRes->SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);                      //SetServiceId
        pPduRes->SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_RESPONSE);  //SetCommandId

        // 添加到响应消息队列中
        CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
    } else {
        log("parse pb failed");
    }
}

}