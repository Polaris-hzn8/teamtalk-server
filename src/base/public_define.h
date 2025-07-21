/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: public_define.h
 Update Time: Tue 13 Jun 2023 14:58:46 CST
 brief: 
*/

#ifndef _public_define_h
#define _public_define_h

#include <set>
#include <iostream>
#include "ostype.h"
#include "IM.BaseDefine.pb.h"
using namespace std;

#define XIAO_T_UID  99999999

enum {
    USER_CNT_INC = 1,
    USER_CNT_DEC = 2,
};

enum {
    IM_GROUP_SETTING_PUSH = 1,
};

enum {
    IM_PUSH_TYPE_NORMAL = 1,
    IM_PUSH_TYPE_SILENT = 2,
};

enum {
    IM_PC_LOGIN_STATUS_ON = 1,
    IM_PC_LOGIN_STATUS_OFF = 0,
};

// client type:
#define CLIENT_TYPE_FLAG_NONE    0x00
#define CLIENT_TYPE_FLAG_PC      0x01
#define CLIENT_TYPE_FLAG_MOBILE  0x02
#define CLIENT_TYPE_FLAG_BOTH    0x03


// enum {
//     CLIENT_TYPE_WINDOWS     = 0x01,
//     CLIENT_TYPE_MAC         = 0x02,
//     CLIENT_TYPE_IOS         = 0x11,
//     CLIENT_TYPE_ANDROID     = 0x12,
// };


///////////////////////////////////CHECK_CLIENT_TYPE_PC/////////////////////////
#define CHECK_CLIENT_TYPE_PC(type) \
({\
    bool bRet = false;\
    if ((type & 0x10) == 0x00)\
    {\
        bRet = true;\
    }\
    bRet;\
})

#define CHECK_CLIENT_TYPE_MOBILE(type) \
({\
    bool bRet = false;\
    if ((type & 0x10) == 0x10)\
    {\
        bRet = true;\
    }\
    bRet;\
})

enum {
    GENDER_UNKNOWN  = 0,
    GENDER_MAN      = 1,
    GENDER_WOMAN    = 2,
};

// enum {
//     SESSION_TYPE_SINGLE     = 0x01,
//     SESSION_TYPE_GROUP      = 0x02,
// };

// enum {
//     MSG_TYPE_SINGLE_TEXT    = 0x01,
//     MSG_TYPE_SINGLE_AUDIO   = 0x02,
//     MSG_TYPE_GROUP_TEXT     = 0x11,
//     MSG_TYPE_GROUP_AUDIO    = 0x12,
// };


///////////////////////////////////////////////CHECK_MSG_TYPE_SINGLE//////////////////////////////////////////////////
#define CHECK_MSG_TYPE_SINGLE(type) \
({\
bool bRet = false;\
if ((IM::BaseDefine::MSG_TYPE_SINGLE_TEXT == type) || (IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO == type))\
{\
bRet = true;\
}\
bRet;\
})


#define CHECK_MSG_TYPE_GROUP(type) \
({\
bool bRet = false;\
if ((IM::BaseDefine::MSG_TYPE_GROUP_TEXT == type) || (IM::BaseDefine::MSG_TYPE_GROUP_AUDIO == type))\
{\
bRet = true;\
}\
bRet;\
})


////////////////////////////////////////定义结构体//////////////////////////////////////////////
typedef struct AudioMsgInfo{
    uint32_t    audioId;
    uint32_t    fileSize;
    uint32_t    data_len;
    uchar_t*    data;
    string      path;
} AudioMsgInfo_t;

// 用户基本信息
typedef struct DBUserInfo_t {
    /* 成员变量 */
    uint32_t    nId;        // 用户ID
    uint8_t     nSex;       // 用户性别 1.男;2.女
    uint8_t     nStatus;    // 用户状态0 正常， 1 离职
    uint32_t    nDeptId;    // 所属部门
    string      strNick;    // 花名
    string      strDomain;  // 花名拼音
    string      strName;    // 真名
    string      strTel;     // 手机号码
    string      strEmail;   // Email
    string      strAvatar;  // 头像
    string      sign_info;  // 个性签名

    /* 赋值运算符重载函数 用于DBUserInfo_t对象之间的赋值操作 */
    DBUserInfo_t& operator=(const DBUserInfo_t& rhs) {
        /* 避免自我赋值 */
        if(this != &rhs) {
            nId = rhs.nId;
            nSex = rhs.nSex;
            nStatus = rhs.nStatus;
            nDeptId = rhs.nDeptId;
            strNick = rhs.strNick;
            strDomain = rhs.strDomain;
            strName = rhs.strName;
            strTel = rhs.strTel;
            strEmail = rhs.strEmail;
            strAvatar = rhs.strAvatar;
            sign_info = rhs.sign_info;
        }
        return *this;
    }
} DBUserInfo_t;
typedef hash_map<uint32_t, DBUserInfo_t*> DBUserMap_t;

// 部门基本信息
typedef struct DBDeptInfo_t {
    /* 成员变量 */
    uint32_t    nId;
    uint32_t    nParentId;
    string      strName;
    
    /* 赋值运算符重载函数 用于DBDeptInfo_t对象之间的赋值操作 */
    DBDeptInfo_t& operator=(const DBDeptInfo_t& rhs) {
        /* 避免自我赋值 */
        if(this != &rhs) {
            nId = rhs.nId;
            nParentId = rhs.nParentId;
            strName = rhs.strName;
        }
        return *this;
    }
    
} DBDeptInfo_t;
typedef hash_map<uint32_t, DBDeptInfo_t*> DBDeptMap_t;

// 用户连接信息
typedef struct {
    uint32_t 	user_id;    //用户id
    uint32_t	conn_cnt;   //连接数量
} user_conn_t;


// 用户状态信息
typedef struct {
    uint32_t user_id;       //用户ID
    uint32_t status;        //用户状态
    uint32_t client_type;   //登录终端类型
} user_stat_t;

// 授权信息
typedef struct {
    uint32_t        user_id;            //用户id
    set<uint32_t>   allow_user_ids;     //允许访问的用户id集合
    set<uint32_t>   allow_group_ids;    //允许访问的用户组id集合
    set<string>     authed_ips;         //已授权的IP地址集合
    set<string>     authed_interfaces;  //已授权的接口名称集合
} auth_struct;

#define MAX_MSG_LEN     4096

#endif

