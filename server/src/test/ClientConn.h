/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ClientConn.h
 Update Time: Thu 15 Jun 2023 01:01:17 CST
 brief:
*/

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "IM.Group.pb.h"
#include "IM.Login.pb.h"
#include "IM.Message.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IPacketCallback.h"
#include "SeqAlloctor.h"
#include "imconn.h"

class ClientConn : public CImConn {
public:
    ClientConn(IPacketCallback* pClient);
    virtual ~ClientConn();

    bool IsOpen() { return m_bOpen; }

    net_handle_t connect(const string& strIp, uint16_t nPort, const string& strName, const string& strPass);

    virtual void Close();

public:
    uint32_t login(const string& strName, const string& strPass);
    uint32_t getUser(uint32_t nUserId, uint32_t nTime = 0);
    uint32_t getUserInfo(uint32_t nUserId, list<uint32_t>& lsUserId);
    uint32_t sendMessage(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, const string& strMsgData);
    uint32_t getUnreadMsgCnt(uint32_t nUserId);
    uint32_t getRecentSession(uint32_t nUserId, uint32_t nLastTime);
    uint32_t getMsgList(uint32_t nUserId, IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt);
    uint32_t sendMsgAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId);
    uint32_t registerUser(const string& strName, const string& strNick);

public:
    virtual void OnConfirm();
    virtual void OnClose();
    virtual void OnTimer(uint64_t curr_tick);

    virtual void HandlePdu(CImPdu* pPdu);

private:
    void _HandleLoginResponse(CImPdu* pPdu);
    void _HandleUser(CImPdu* pPdu);
    void _HandleUserInfo(CImPdu* pPdu);
    void _HandleSendMsg(CImPdu* pPdu);
    void _HandleUnreadCnt(CImPdu* pPdu);
    void _HandleRecentSession(CImPdu* pPdu);
    void _HandleMsgList(CImPdu* pPdu);
    void _HandleMsgData(CImPdu* pPdu);

private:
    bool m_bOpen;
    IPacketCallback* m_pCallback;
    CSeqAlloctor* m_pSeqAlloctor;
};

#endif /* CLIENTCONN_H_ */
