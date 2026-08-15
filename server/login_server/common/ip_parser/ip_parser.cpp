/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ipparser.cpp
 Update Time: Thu 15 Jun 2023 00:44:59 CST
 brief:
*/

#include <common/ip_parser/ip_parser.h>
#include <teamtalk/imcore/slog/slog.h>
#include <teamtalk/imcore/string/str_explode.h>

namespace teamtalk::login_server::common::ip_parser {

using namespace teamtalk::imcore::string;

bool is_telcome(const char* ip) {
  if (ip == nullptr) {
    log_error("ip is null");
    return false;
  }
  CStrExplode strExp((char*)ip, '.');
  if (strExp.GetItemCnt() != 4) {
    log_error("ip is not valid");
    return false;
  }
  return true;
}

}  // namespace teamtalk::login_server::common::ip_parser
