/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpPdu.h
 Update Time: Thu 15 Jun 2023 00:42:45 CST
 brief:
*/

#ifndef HTTPPDU_H_
#define HTTPPDU_H_

#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"
#include "util.h"
#include <list>

// jsonp parameter parser
class CPostDataParser {
public:
    CPostDataParser() { }
    virtual ~CPostDataParser() { }

    bool Parse(const char* content);

    char* GetValue(const char* key);

private:
    std::map<std::string, std::string> m_post_map;
};

char* PackSendResult(uint32_t error_code, const char* error_msg = "");
char* PackSendCreateGroupResult(uint32_t error_code, const char* error_msg, uint32_t group_id);
char* PackGetUserIdByNickNameResult(uint32_t result, std::list<IM::BaseDefine::UserInfo> user_list);
#endif /* HTTPPDU_H_ */
