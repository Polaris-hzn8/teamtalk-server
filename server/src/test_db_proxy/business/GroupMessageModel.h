/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: GroupMessageModel.h
 Update Time: Thu 15 Jun 2023 01:06:11 CST
 brief:
*/

#ifndef GROUP_MESSAGE_MODEL_H_
#define GROUP_MESSAGE_MODEL_H_

#include <list>
#include <string>

#include "AudioModel.h"
#include "GroupModel.h"
#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"
#include "util.h"

using namespace std;

class CGroupMessageModel {
public:
    virtual ~CGroupMessageModel();
    static CGroupMessageModel* getInstance();

    bool sendMessage(uint32_t nFromId, uint32_t nGroupId, IM::BaseDefine::MsgType nMsgType, uint32_t nCreateTime, uint32_t nMsgId, const string& strMsgContent);
    bool sendAudioMessage(uint32_t nFromId, uint32_t nGroupId, IM::BaseDefine::MsgType nMsgType, uint32_t nCreateTime, uint32_t nMsgId, const char* pMsgContent, uint32_t nMsgLen);
    void getMessage(uint32_t nUserId, uint32_t nGroupId, uint32_t nMsgId, uint32_t nMsgCnt,
        list<IM::BaseDefine::MsgInfo>& lsMsg);
    bool clearMessageCount(uint32_t nUserId, uint32_t nGroupId);
    uint32_t getMsgId(uint32_t nGroupId);
    void getUnreadMsgCount(uint32_t nUserId, uint32_t& nTotalCnt, list<IM::BaseDefine::UnreadInfo>& lsUnreadCount);
    void getLastMsg(uint32_t nGroupId, uint32_t& nMsgId, string& strMsgData, IM::BaseDefine::MsgType& nMsgType, uint32_t& nFromId);
    void getUnReadCntAll(uint32_t nUserId, uint32_t& nTotalCnt);
    void getMsgByMsgId(uint32_t nUserId, uint32_t nGroupId, const list<uint32_t>& lsMsgId, list<IM::BaseDefine::MsgInfo>& lsMsg);
    bool resetMsgId(uint32_t nGroupId);

private:
    CGroupMessageModel();
    bool incMessageCount(uint32_t nUserId, uint32_t nGroupId);

private:
    static CGroupMessageModel* m_pInstance;
};

#endif /* MESSAGE_MODEL_H_ */
