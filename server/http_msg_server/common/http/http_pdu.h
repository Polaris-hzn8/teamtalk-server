/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpPdu.h
 Update Time: Thu 15 Jun 2023 00:42:45 CST
 brief:
*/

#ifndef TEAMTALK_HTTP_SERVER_COMMON_HTTP_HTTP_PDU_H_
#define TEAMTALK_HTTP_SERVER_COMMON_HTTP_HTTP_PDU_H_

#include <list>

#include <teamtalk/imcore/ttidl/base_define.pb.h>

namespace teamtalk::http_server::common::http {

namespace ttidlbase = teamtalk::imcore::ttidl::base_define;

class CPostDataParser {
 public:
  CPostDataParser() {}
  virtual ~CPostDataParser() {}

  bool Parse(const char* content);

  char* GetValue(const char* key);

 private:
  std::map<std::string, std::string> m_post_map;
};

char* PackSendResult(uint32_t error_code, const char* error_msg = "");
char* PackSendCreateGroupResult(uint32_t error_code, const char* error_msg, uint32_t group_id);
char* PackGetUserIdByNickNameResult(uint32_t result, std::list<ttidlbase::UserInfo> user_list);

}  // namespace teamtalk::http_server::common::http

#endif  // TEAMTALK_HTTP_SERVER_COMMON_HTTP_HTTP_PDU_H_