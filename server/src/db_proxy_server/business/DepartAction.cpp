/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DepartAction.cpp
 Update Time: Thu 15 Jun 2023 00:28:55 CST
 brief:
*/

#include "DepartAction.h"
#include "../ProxyConn.h"
#include "DepartModel.h"
#include "IM.Buddy.pb.h"

namespace DB_PROXY {


/// @brief 获取变更的部门信息
/// @param pPdu 指向 CImPdu 对象的指针
/// @param conn_uuid 连接的唯一标识符 conn_uuid
void getChgedDepart(CImPdu* pPdu, uint32_t conn_uuid) {
    // 创建一个空的部门信息响应消息对象 msgResp
    IM::Buddy::IMDepartmentReq msg;
    IM::Buddy::IMDepartmentRsp msgResp;
    // 解析收到的部门信息请求消息 获取用户ID和最新更新时间
    if (msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        // 1.成功解析消息
        // 1-1.创建一个待发送的消息对象 pPduRes
        CImPdu* pPduRes = new CImPdu;
        uint32_t nUserId = msg.user_id();
        uint32_t nLastUpdate = msg.latest_update_time();

        // 1-2.从数据库中获取最新更新时间后发生变化的部门ID列表 lsChangedIds，以及对应的部门信息列表 lsDeparts
        list<uint32_t> lsChangedIds;
        CDepartModel::getInstance()->getChgedDeptId(nLastUpdate, lsChangedIds);
        list<IM::BaseDefine::DepartInfo> lsDeparts;
        CDepartModel::getInstance()->getDepts(lsChangedIds, lsDeparts);

        // 1-3.设置部门信息响应消息的用户ID和最新更新时间
        msgResp.set_user_id(nUserId);
        msgResp.set_latest_update_time(nLastUpdate);

        // 1-4.遍历部门信息列表，将每个部门信息添加到部门信息响应消息中
        for (auto it = lsDeparts.begin(); it != lsDeparts.end(); ++it) {
            IM::BaseDefine::DepartInfo* pDeptInfo = msgResp.add_dept_list();
            pDeptInfo->set_dept_id(it->dept_id());
            pDeptInfo->set_priority(it->priority());
            pDeptInfo->set_dept_name(it->dept_name());
            pDeptInfo->set_parent_dept_id(it->parent_dept_id());
            pDeptInfo->set_dept_status(it->dept_status());
        }
        log("userId=%u, last_update=%u, cnt=%u", nUserId, nLastUpdate, lsDeparts.size());

        // 1-5.设置附加数据为请求消息中的附加数据
        msgResp.set_attach_data(msg.attach_data());//设置附加消息
        pPduRes->SetPBMsg(&msgResp);//设置消息体
        pPduRes->SetSeqNum(pPdu->GetSeqNum());//设置消息序号
        pPduRes->SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);//SetServiceId
        pPduRes->SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_DEPARTMENT_RESPONSE);//SetCommandId CID_BUDDY_LIST_DEPARTMENT_RESPONSE

        // 1-6.将待发送的消息对象 pPduRes 添加到响应消息队列中，以便后续发送给客户端
        CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
    } else {
        // 2.解析消息失败 直接记录日志
        log("parse pb failed");
    }
}

}