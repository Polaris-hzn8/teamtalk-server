/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Login.cpp
 Update Time: Thu 15 Jun 2023 00:31:17 CST
 brief:
*/

#include <list>
#include <unordered_map>
#include "Login.h"
#include "Base64.h"
#include "Common.h"
#include "json/json.h"
#include "UserModel.h"
#include "InterLogin.h"
#include "ExterLogin.h"
#include "IM.Server.pb.h"
#include "TokenValidator.h"
#include "HttpClient.h"
#include "ProxyConn.h"
#include "SyncCenter.h"

namespace DB_PROXY {

CInterLoginStrategy g_loginStrategy;

CLock g_cLimitLock;
std::unordered_map<std::string, std::list<uint32_t>> g_hmLimits;

void doLogin(CImPdu* pPdu, uint32_t conn_uuid)
{
    CImPdu* pPduResp = new CImPdu;

    IM::Server::IMValidateReq msg;
    IM::Server::IMValidateRsp msgResp;
    if (msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        std::string strDomain = msg.user_name(); //用户名
        std::string strPass = msg.password();    //密码

        msgResp.set_user_name(strDomain);
        msgResp.set_attach_data(msg.attach_data());

        // 登录次数限制检查 30分钟内
        do {
            CAutoLock cAutoLock(&g_cLimitLock);

            // 获取对应用户名的错误登录时间列表
            std::list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];

            // 清理超过30分钟的错误登录时间点记录
            uint32_t tmNow = time(NULL);
            auto itTime = lsErrorTime.begin();
            for (; itTime != lsErrorTime.end(); ++itTime)
                if (tmNow - *itTime > 30 * 60)
                    break;
        
            //清理放在这里还是放在密码错误后添加的时候呢？
            //放在这里，每次都要遍历，会有一点点性能的损失。
            //放在后面，可能会造成30分钟之前有10次错的，但是本次是对的就没办法再访问了
            
            if (itTime != lsErrorTime.end())
                lsErrorTime.erase(itTime, lsErrorTime.end());

            // 判断30分钟内密码错误次数是否大于10
            if (lsErrorTime.size() > 10) {
                itTime = lsErrorTime.begin();
                if (tmNow - *itTime <= 30 * 60) {
                    msgResp.set_result_code(6);
                    msgResp.set_result_string("用户名/密码错误次数太多");
                    pPduResp->SetPBMsg(&msgResp);
                    pPduResp->SetSeqNum(pPdu->GetSeqNum());
                    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);
                    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);
                    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
                    return;
                }
            }
        } while (false);

        log("%s request login.", strDomain.c_str());

        // 登录验证
        IM::BaseDefine::UserInfo cUser;
        if (g_loginStrategy.doLogin(strDomain, strPass, cUser)) {
            /* 登录成功密码信息正确 */
            IM::BaseDefine::UserInfo* pUser = msgResp.mutable_user_info();
            pUser->set_user_id(cUser.user_id());
            pUser->set_user_gender(cUser.user_gender());
            pUser->set_department_id(cUser.department_id());
            pUser->set_user_nick_name(cUser.user_nick_name());
            pUser->set_user_domain(cUser.user_domain());
            pUser->set_avatar_url(cUser.avatar_url());

            pUser->set_email(cUser.email());
            pUser->set_user_tel(cUser.user_tel());
            pUser->set_user_real_name(cUser.user_real_name());
            pUser->set_status(0);

            pUser->set_sign_info(cUser.sign_info());//用户个性签名

            msgResp.set_result_code(0);
            msgResp.set_result_string("成功");

            CAutoLock cAutoLock(&g_cLimitLock);
            std::list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.clear();
        } else {
            /* 登录失败密码信息有误 */
            uint32_t tmCurrent = time(NULL);

            CAutoLock cAutoLock(&g_cLimitLock);
            std::list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.push_front(tmCurrent);

            msgResp.set_result_code(1);
            msgResp.set_result_string("用户名/密码错误");

            log("get result false");
        }
    } else {
        msgResp.set_result_code(2);
        msgResp.set_result_string("服务端内部错误");
    }
    // 登录响应消息回发
    pPduResp->SetPBMsg(&msgResp);                                   //设置消息体
    pPduResp->SetSeqNum(pPdu->GetSeqNum());                         //设置消息序号
    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);              //SetServiceId
    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP); //command_id
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);                //AddResponsePdu
}

}
