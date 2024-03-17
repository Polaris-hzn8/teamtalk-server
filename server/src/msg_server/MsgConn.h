/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: MsgConn.h
 Update Time: Thu 15 Jun 2023 00:56:58 CST
 brief:
*/

#ifndef MSGCONN_H_
#define MSGCONN_H_

#include "imconn.h"

#define KICK_FROM_ROUTE_SERVER 1
#define MAX_ONLINE_FRIEND_CNT 100 // 通知好友状态通知的最多个数

typedef struct {
    uint32_t msg_id;//消息ID
    uint32_t from_id;//消息发送者的ID
    uint64_t timestamp;//消息的时间戳
} msg_ack_t;

class CImUser;

class CMsgConn : public CImConn {
public:
    CMsgConn();
    virtual ~CMsgConn();
    
    //Getter和Setter函数 用于获取和设置类中的成员变量，如登录名、用户ID、连接句柄等
    string GetLoginName() { return m_login_name; }
    uint32_t GetUserId() { return m_user_id; }
    void SetUserId(uint32_t user_id) { m_user_id = user_id; }
    uint32_t GetHandle() { return m_handle; }
    uint16_t GetPduVersion() { return m_pdu_version; }
    uint32_t GetClientType() { return m_client_type; }
    uint32_t GetClientTypeFlag();
    void SetOpen() { m_bOpen = true; }
    bool IsOpen() { return m_bOpen; }
    void SetKickOff() { m_bKickOff = true; }
    bool IsKickOff() { return m_bKickOff; }
    void SetOnlineStatus(uint32_t status) { m_online_status = status; }
    uint32_t GetOnlineStatus() { return m_online_status; }

    //发送用户状态更新
    void SendUserStatusUpdate(uint32_t user_status);
    //关闭连接 可以选择是否踢出用户
    virtual void Close(bool kick_user = false);

    //连接建立时的回调函数
    virtual void OnConnect(net_handle_t handle);
    //连接关闭时的回调函数
    virtual void OnClose();
    //定时器触发时的回调函数
    virtual inline void OnTimer(uint64_t curr_tick);

    //处理收到的PDU 根据PDU的命令ID，将其分派给相应的处理函数
    virtual void HandlePdu(CImPdu* pPdu);

    //将消息ID和发送者ID添加到发送列表中
    void AddToSendList(uint32_t msg_id, uint32_t from_id);
    //从发送列表中删除消息ID和发送者ID
    void DelFromSendList(uint32_t msg_id, uint32_t from_id);

private:
    //私有函数 用于处理不同类型的PDU 根据PDU的命令ID来执行相应的逻辑
    void _HandleHeartBeat(CImPdu* pPdu);
    void _HandleLoginRequest(CImPdu* pPdu);
    void _HandleLoginOutRequest(CImPdu* pPdu);
    void _HandleClientRecentContactSessionRequest(CImPdu* pPdu);
    void _HandleClientMsgData(CImPdu* pPdu);
    void _HandleClientMsgDataAck(CImPdu* pPdu);
    void _HandleClientTimeRequest(CImPdu* pPdu);
    void _HandleClientGetMsgListRequest(CImPdu* pPdu);
    void _HandleClientGetMsgByMsgIdRequest(CImPdu* pPdu);
    void _HandleClientUnreadMsgCntRequest(CImPdu* pPdu);
    void _HandleClientMsgReadAck(CImPdu* pPdu);
    void _HandleClientGetLatestMsgIDReq(CImPdu* pPdu);
    void _HandleClientP2PCmdMsg(CImPdu* pPdu);
    void _HandleClientUserInfoRequest(CImPdu* pPdu);
    void _HandleClientUsersStatusRequest(CImPdu* pPdu);
    void _HandleClientRemoveSessionRequest(CImPdu* pPdu);
    void _HandleClientAllUserRequest(CImPdu* pPdu);
    void _HandleChangeAvatarRequest(CImPdu* pPdu);
    void _HandleChangeSignInfoRequest(CImPdu* pPdu);

    void _HandleClientDeviceToken(CImPdu* pPdu);
    void _HandleKickPCClient(CImPdu* pPdu);
    void _HandleClientDepartmentRequest(CImPdu* pPdu);
    void _SendFriendStatusNotify(uint32_t status);
    void _HandlePushShieldRequest(CImPdu* pPdu);
    void _HandleQueryPushShieldRequest(CImPdu* pPdu);
    void _HandleRegistRequest(CImPdu* pPdu);

private:
    string m_login_name;        //登录名拼音，用于存储用户的登录名
    uint32_t m_user_id;         //用户ID
    bool m_bOpen;               //连接是否已打开的标志 当通过数据库验证后，该标志将设置为 true
    bool m_bKickOff;            //连接是否被踢出的标志 当用户被强制下线时，该标志将设置为true
    uint64_t m_login_time;      //用户登录时间的时间戳

    uint32_t m_last_seq_no;     //最后一个序列号，用于消息的顺序处理
    uint16_t m_pdu_version;     //PDU（协议数据单元）的版本号
    string m_client_version;    //客户端的版本信息，例如 MAC/2.2 或 WIN/2.2

    list<msg_ack_t> m_send_msg_list;    //发送消息列表，用于存储待发送的消息的消息ID和发送者ID
    uint32_t m_msg_cnt_per_sec;         //每秒发送的消息数
    uint32_t m_client_type;             //客户端登录方式
    uint32_t m_online_status;           //在线状态 1-online, 2-off-line, 3-leave
};


void init_msg_conn();

#endif
