/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpParserWrapper.h
 Update Time: Mon 12 Jun 2023 23:46:35 CST
 brief: extract url and content body from an ajax request
*/

#ifndef _HTTP_PARSER_WARPPER_H_
#define _HTTP_PARSER_WARPPER_H_

#include "common/util.h"
#include "http_parser.h"

class CHttpParserWrapper
{
public:
    CHttpParserWrapper();
    virtual ~CHttpParserWrapper() {}
    
    void ParseHttpContent(const char* buf, uint32_t len);

    bool IsReadAll() { return m_read_all; }
    bool IsReadReferer() { return m_read_referer; }
    bool HasReadReferer() { return m_referer.size() > 0; }

    bool IsReadForwardIP() { return m_read_forward_ip; }
    bool HasReadForwardIP() { return m_forward_ip.size() > 0; }

    bool IsReadUserAgent() { return m_read_user_agent; }
    bool HasReadUserAgent() { return m_user_agent.size() > 0; }

    bool IsReadContentType() { return m_read_content_type;  }
    bool HasReadContentType() { return m_content_type.size() > 0; }

    bool IsReadContentLen()  { return m_read_content_len; }
    bool HasReadContentLen() { return m_content_len != 0; }

    bool IsReadHost()  { return m_read_host;  }
    bool HasReadHost() { return m_host.size()>0; }

    // total_length
    uint32_t GetTotalLength() { return m_total_length; }
    void SetTotalLength(uint32_t total_len) { m_total_length = total_len; }

    // url
    char* GetUrl() { return (char*)m_url.c_str(); }
    void SetUrl(const char* url, size_t length) { m_url.append(url, length); }

    // body_content
    char* GetBodyContent() { return (char*)m_body_content.c_str(); }
    uint32_t GetBodyContentLen() { return (uint32_t)m_body_content.length(); }
    void SetBodyContent(const char* content, size_t length) { m_body_content.append(content, length); }

    // referer
    char* GetReferer() { return (char*)m_referer.c_str(); }
    void SetReferer(const char* referer, size_t length) { m_referer.append(referer, length); }

    // forward_ip
    char* GetForwardIP() { return (char*)m_forward_ip.c_str(); }
    void SetForwardIP(const char* forward_ip, size_t length) { m_forward_ip.append(forward_ip, length); }

    // user_agent
    char* GetUserAgent() { return (char*)m_user_agent.c_str(); }
    void SetUserAgent(const char* user_agent, size_t length) { m_user_agent.append(user_agent, length); }

    // content_type
    char* GetContentType() { return (char*) m_content_type.c_str(); }
    void SetContentType(const char* content_type, size_t length) {  m_content_type.append(content_type, length); }

    // content_len
    uint32_t  GetContentLen() { return m_content_len; }
    void SetContentLen(uint32_t content_len) { m_content_len = content_len; }
    
    // host
    char* GetHost() { return (char*) m_host.c_str(); }
    void SetHost(const char* host, size_t length) { m_host.append(host, length); }
    
    // read_content
    void SetReadContentType(bool read_content_type) { m_read_content_type =  read_content_type; }
    void SetReadContentLen(bool read_content_len) { m_read_content_len = read_content_len; }

    // read_host
    void SetReadHost(bool read_host) { m_read_host = read_host; }
    void SetReadAll() { m_read_all = true; }
    void SetReadReferer(bool read_referer) { m_read_referer = read_referer; }
    void SetReadForwardIP(bool read_forward_ip) { m_read_forward_ip = read_forward_ip; }
    void SetReadUserAgent(bool read_user_agent) { m_read_user_agent = read_user_agent; }
    
    char GetMethod() { return (char) m_http_parser.method; }

    // 解析过程回调函数
    static int OnUrl(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeaderField(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeaderValue(http_parser* parser, const char *at, size_t length, void* obj);
    static int OnHeadersComplete (http_parser* parser, void* obj);
    static int OnBody (http_parser* parser, const char *at, size_t length, void* obj);
    static int OnMessageComplete (http_parser* parser, void* obj);

private:
    http_parser m_http_parser;              //HTTP解析器对象，用于解析HTTP请求
    http_parser_settings m_settings;        //HTTP解析器的设置，包含回调函数等配置信息

    bool m_read_all;                        //是否已读取全部的字段
    bool m_read_referer;                    //是否已读取Referer字段
    bool m_read_forward_ip;                 //是否已读取Forward IP字段
    bool m_read_user_agent;                 //是否已读取User-Agent字段
    bool m_read_content_type;               //是否已读取Content-Type字段
    bool m_read_content_len;                //是否已读取Content-Length字段
    bool m_read_host;                       //是否已读取Host字段

    uint32_t m_total_length;                //HTTP请求的总长度
    std::string	m_url;                      //请求的URL
    std::string	m_body_content;             //请求的消息体内容
    std::string	m_referer;                  //Referer字段的值，表示请求的来源
    std::string m_forward_ip;               //请求经过的代理服务器的IP
    std::string m_user_agent;               //发起请求的用户代理(浏览器、应用程序等)
    std::string m_content_type;             //请求消息体的类型
    uint32_t  m_content_len;                //请求消息体的长度
    std::string m_host;                     //请求的目标主机
};

#endif // _HTTP_PARSER_WARPPER_H_
