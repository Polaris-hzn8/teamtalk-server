/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Login.cpp
 Update Time: Thu 15 Jun 2023 00:31:17 CST
 brief:
*/

#include "Login.h"
#include "../HttpClient.h"
#include "../ProxyConn.h"
#include "../SyncCenter.h"
#include "Base64.h"
#include "Common.h"
#include "ExterLogin.h"
#include "IM.Server.pb.h"
#include "InterLogin.h"
#include "TokenValidator.h"
#include "UserModel.h"
#include "json/json.h"
#include <list>

CInterLoginStrategy g_loginStrategy;

hash_map<string, list<uint32_t>> g_hmLimits;
CLock g_cLimitLock;
namespace DB_PROXY {

void doLogin(CImPdu* pPdu, uint32_t conn_uuid) {
    //创建一个用于响应的CImPdu对象pPduResp
    CImPdu* pPduResp = new CImPdu;
    //创建一个登录响应消息msgResp
    IM::Server::IMValidateRsp msgResp;
    IM::Server::IMValidateReq msg;

    // 1.解析登录请求中的用户名和密码 并将其存储到 strDomain 和 strPass 变量中
    if (msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength())) {
        string strDomain = msg.user_name();//用户名
        string strPass = msg.password();//密码

        // 2.创建一个登录响应消息msgResp 设置用户名和附加数据
        msgResp.set_user_name(strDomain);
        msgResp.set_attach_data(msg.attach_data());

        // 3.检查登录限制
        do {
            // 3-1.使用 g_cLimitLock 对登录限制进行加锁
            CAutoLock cAutoLock(&g_cLimitLock);
            // 3-2.从 g_hmLimits 哈希表中获取对应用户名的错误登录时间列表 lsErrorTime
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];

            // 3-3.清理超过30分钟的错误登录时间点记录
            uint32_t tmNow = time(NULL);
            auto itTime = lsErrorTime.begin();
            for (; itTime != lsErrorTime.end(); ++itTime) {
                if (tmNow - *itTime > 30 * 60) {
                    break;
                }
            }
        
            //清理放在这里还是放在密码错误后添加的时候呢？
            //放在这里，每次都要遍历，会有一点点性能的损失。
            //放在后面，可能会造成30分钟之前有10次错的，但是本次是对的就没办法再访问了
            
            if (itTime != lsErrorTime.end()) {
                lsErrorTime.erase(itTime, lsErrorTime.end());
            }

            // 3-4.判断30分钟内密码错误次数是否大于10
            if (lsErrorTime.size() > 10) {
                itTime = lsErrorTime.begin();
                if (tmNow - *itTime <= 30 * 60) {
                    // 如果超过限制，则返回登录失败响应
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

        // 4.记录登录请求日志
        log("%s request login.", strDomain.c_str());

        // 5.调用 g_loginStrategy 的doLogin()函数进行实际的登录验证，传入用户名、密码和用户信息参数
        IM::BaseDefine::UserInfo cUser;
        if (g_loginStrategy.doLogin(strDomain, strPass, cUser)) {
            // 5-1.如果登录验证成功，将用户信息填充到登录响应消息中，并设置登录成功的结果码和结果字符串
            IM::BaseDefine::UserInfo* pUser = msgResp.mutable_user_info();
            pUser->set_user_id(cUser.user_id());//uid
            pUser->set_user_gender(cUser.user_gender());//gender
            pUser->set_department_id(cUser.department_id());//department
            pUser->set_user_nick_name(cUser.user_nick_name());//nick_name
            pUser->set_user_domain(cUser.user_domain());//domain
            pUser->set_avatar_url(cUser.avatar_url());//avator_url

            pUser->set_email(cUser.email());//email
            pUser->set_user_tel(cUser.user_tel());//tel
            pUser->set_user_real_name(cUser.user_real_name());//real_name
            pUser->set_status(0);//status

            pUser->set_sign_info(cUser.sign_info());//用户个性签名

            msgResp.set_result_code(0);//result_code
            msgResp.set_result_string("成功");//result_string

            // 如果登陆成功，则清除错误尝试限制
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.clear();
        } else {
            // 5-2.如果登录验证失败，记录一次登录失败，并设置登录失败的结果码和结果字符串
            // 获取当前时间戳 tmCurrent，用于记录错误登录的时间
            uint32_t tmCurrent = time(NULL);
            // 使用互斥锁 g_cLimitLock 对登录限制进行加锁，以确保多线程环境下的数据安全性
            CAutoLock cAutoLock(&g_cLimitLock);
            // 通过用户名 strDomain 在 g_hmLimits 哈希表中获取对应的错误登录时间列表 lsErrorTime
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            // 将当前时间戳 tmCurrent 插入到错误登录时间列表 lsErrorTime 的头部，表示发生了一次错误登录
            lsErrorTime.push_front(tmCurrent);

            // 输出日志信息，提示用户名/密码错误
            log("get result false");
            // 设置登录响应消息的结果码为 1，表示用户名/密码错误
            msgResp.set_result_code(1);
            msgResp.set_result_string("用户名/密码错误");
        }
    } else {
        // 6.如果登录请求解析失败，设置内部错误的结果码和结果字符串
        msgResp.set_result_code(2);
        msgResp.set_result_string("服务端内部错误");
    }

    // 7.将登录响应消息发送回客户端
    pPduResp->SetPBMsg(&msgResp);//设置消息体
    pPduResp->SetSeqNum(pPdu->GetSeqNum());//设置消息序号
    pPduResp->SetServiceId(IM::BaseDefine::SID_OTHER);//SetServiceId
    pPduResp->SetCommandId(IM::BaseDefine::CID_OTHER_VALIDATE_RSP);//设置新的command_id CID_OTHER_VALIDATE_RSP !!!!!!!!!!
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);//AddResponsePdu
}

};
