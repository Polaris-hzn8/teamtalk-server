/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpParserWrapper.cpp
 Update Time: Mon 12 Jun 2023 23:46:28 CST
 brief:
*/

#include "HttpParserWrapper.h"
#include "http_parser.h"

#define MAX_REFERER_LEN 32

CHttpParserWrapper::CHttpParserWrapper() { }

void CHttpParserWrapper::ParseHttpContent(const char* buf, uint32_t len) {
    //初始化 m_http_parser 和 m_settings 对象
    http_parser_init(&m_http_parser, HTTP_REQUEST);
    memset(&m_settings, 0, sizeof(m_settings));

    //设置解析器的回调函数
    m_settings.on_url = OnUrl;
    m_settings.on_header_field = OnHeaderField;
    m_settings.on_header_value = OnHeaderValue;
    m_settings.on_headers_complete = OnHeadersComplete;
    m_settings.on_body = OnBody;
    m_settings.on_message_complete = OnMessageComplete;

    //将当前对象的指针设置为 m_settings 的 object 字段，以便在回调函数中可以访问到当前对象的成员变量和方法
    m_settings.object = this;

    //重置解析器相关的标志和成员变量，包括是否已读取全部字段
    m_read_all = false;
    m_read_referer = false;
    m_read_forward_ip = false;
    m_read_user_agent = false;
    m_read_content_type = false;
    m_read_content_len = false;
    m_read_host = false;
    m_total_length = 0;
    m_url.clear();
    m_body_content.clear();
    m_referer.clear();
    m_forward_ip.clear();
    m_user_agent.clear();
    m_content_type.clear();
    m_content_len = 0;
    m_host.clear();

    //调用 http_parser_execute 函数
    //传入解析器、设置、HTTP内容的缓冲区 buf 和长度 len参数 该函数会根据设置的回调函数逐步解析 HTTP 内容
    http_parser_execute(&m_http_parser, &m_settings, buf, len);
}

/**
 * 在解析HTTP请求的URL时，该函数将解析得到的URL字符串设置到 CHttpParserWrapper 对象的成员变量中
 * 这样解析器在解析完整个 URL 后，就可以通过调用 GetUrl() 方法来获取解析得到的 URL 字符串
*/
int CHttpParserWrapper::OnUrl(http_parser* parser, const char* at, size_t length, void* obj) {
    //将解析得到的 URL 字符串 at（指针）和长度 length 作为参数传入
    //将 URL 字符串设置到 CHttpParserWrapper 对象的成员变量 m_url 中
    ((CHttpParserWrapper*)obj)->SetUrl(at, length);
    return 0;
}

/**
 * 在解析HTTP头部字段时，会根据字段名称判断是否需要读取该字段的值，并设置相应的标志位
*/
int CHttpParserWrapper::OnHeaderField(http_parser* parser, const char* at, size_t length, void* obj) {
    //检查是否已经读取过 Referer 字段
    if (!((CHttpParserWrapper*)obj)->HasReadReferer()) {
        //如果还未读取，并且当前字段的名称是 "Referer"，则将读取Referer标志位设置为 true
        if (strncasecmp(at, "Referer", 7) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadReferer(true);
        }
    }

    //检查是否已经读取过 X-Forwarded-For 字段
    if (!((CHttpParserWrapper*)obj)->HasReadForwardIP()) {
        //如果还未读取，并且当前字段的名称是 "X-Forwarded-For"，则将读取 X-Forwarded-For 标志位设置为 true
        if (strncasecmp(at, "X-Forwarded-For", 15) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadForwardIP(true);
        }
    }

    //检查是否已经读取过 User-Agent 字段
    if (!((CHttpParserWrapper*)obj)->HasReadUserAgent()) {
        //如果还未读取，并且当前字段的名称是 "User-Agent"，则将读取 User-Agent 标志位设置为 true
        if (strncasecmp(at, "User-Agent", 10) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadUserAgent(true);
        }
    }

    //检查是否已经读取过 Content-Type 字段
    if (!((CHttpParserWrapper*)obj)->HasReadContentType()) {
        //如果还未读取，并且当前字段的名称是 "Content-Type"，则将读取 Content-Type 标志位设置为 true
        if (strncasecmp(at, "Content-Type", 12) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadContentType(true);
        }
    }

    //检查是否已经读取过 Content-Length 字段
    if (!((CHttpParserWrapper*)obj)->HasReadContentLen()) {
        //如果还未读取，并且当前字段的名称是 "Content-Length"，则将读取 Content-Length 标志位设置为 true
        if (strncasecmp(at, "Content-Length", 14) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadContentLen(true);
        }
    }

    //检查是否已经读取过 Host 字段
    if (!((CHttpParserWrapper*)obj)->HasReadHost()) {
        //如果还未读取，并且当前字段的名称是 "Host"，则将读取 Host 标志位设置为 true
        if (strncasecmp(at, "Host", 4) == 0) {
            ((CHttpParserWrapper*)obj)->SetReadHost(true);
        }
    }
    return 0;
}

/**
 * 该函数在解析器解析到请求头的值时进行回调
*/
int CHttpParserWrapper::OnHeaderValue(http_parser* parser, const char* at, size_t length, void* obj) {
    //判断是否需要解析 Referer 字段
    if (((CHttpParserWrapper*)obj)->IsReadReferer()) {
        size_t referer_len = (length > MAX_REFERER_LEN) ? MAX_REFERER_LEN : length;
        //如果需要 则将请求头值的内容（at）以最大长度 MAX_REFERER_LEN（如果长度超过该限制）或实际长度作为参数调用 SetReferer 方法
        ((CHttpParserWrapper*)obj)->SetReferer(at, referer_len);
        //将解析到的 Referer 值设置到 m_referer 成员变量中，并将 m_read_referer 设置为 false，表示已读取完 Referer 字段
        ((CHttpParserWrapper*)obj)->SetReadReferer(false);
    }

    //判断是否需要解析 ForwardIP 字段
    if (((CHttpParserWrapper*)obj)->IsReadForwardIP()) {
        //将请求头值的内容（at）以实际长度作为参数调用 SetForwardIP 方法，将解析到的 ForwardIP 值设置到 m_forward_ip 成员变量中
        ((CHttpParserWrapper*)obj)->SetForwardIP(at, length);
        //将m_read_forward_ip 设置为 false，表示已读取完 ForwardIP 字段
        ((CHttpParserWrapper*)obj)->SetReadForwardIP(false);
    }

    //判断是否需要解析 User-Agent 字段
    if (((CHttpParserWrapper*)obj)->IsReadUserAgent()) {
        //将请求头值的内容（at）以实际长度作为参数调用 SetUserAgent 方法，将解析到的 User-Agent 值设置到 m_user_agent 成员变量中
        ((CHttpParserWrapper*)obj)->SetUserAgent(at, length);
        //将 m_read_user_agent 设置为 false，表示已读取完 User-Agent 字段
        ((CHttpParserWrapper*)obj)->SetReadUserAgent(false);
    }

    //判断是否需要解析 Content-Type 字段
    if (((CHttpParserWrapper*)obj)->IsReadContentType()) {
        //将请求头值的内容（at）以实际长度作为参数调用 SetContentType 方法，将解析到的 Content-Type 值设置到 m_content_type 成员变量中
        ((CHttpParserWrapper*)obj)->SetContentType(at, length);
        //将 m_read_content_type 设置为 false，表示已读取完 Content-Type 字段
        ((CHttpParserWrapper*)obj)->SetReadContentType(false);
    }

    //判断是否需要解析 Content-Length 字段
    if (((CHttpParserWrapper*)obj)->IsReadContentLen()) {
        string strContentLen(at, length);
        //将请求头值的内容（at）转换为字符串，并使用 atoi 函数将其转换为整数类型，将解析到的 Content-Length 值设置到 m_content_len 成员变量中
        ((CHttpParserWrapper*)obj)->SetContentLen(atoi(strContentLen.c_str()));
        //将m_read_content_len设置为false，表示已读取完Content-Length字段
        ((CHttpParserWrapper*)obj)->SetReadContentLen(false);
    }

    //判断是否需要解析 Host 字段
    if (((CHttpParserWrapper*)obj)->IsReadHost()) {
        //将请求头值的内容（at）以实际长度作为参数调用 SetHost 方法，将解析到的 Host 值设置到 m_host 成员变量中
        ((CHttpParserWrapper*)obj)->SetHost(at, length);
        //将 m_read_host 设置为 false，表示已读取完 Host 字段
        ((CHttpParserWrapper*)obj)->SetReadHost(false);
    }
    return 0;
}

/**
 * 该函数在解析器完成解析请求头时进行回调
 * 目的是在解析器完成解析请求头时更新 CHttpParserWrapper 对象的状态，将解析到的请求头的总长度保存起来
*/
int CHttpParserWrapper::OnHeadersComplete(http_parser* parser, void* obj) {
    //将obj转换为CHttpParserWrapper*类型，并调用SetTotalLength方法，将解析到的请求头的总长度设置到 m_total_length 成员变量中
    ((CHttpParserWrapper*)obj)->SetTotalLength(parser->nread + (uint32_t)parser->content_length);
    return 0;
}

/**
 * 该函数在解析器解析消息体时进行回调
 * 这个回调函数的目的是在解析器解析消息体时更新 CHttpParserWrapper 对象的状态
 * 将解析到的消息体内容保存起来
*/
int CHttpParserWrapper::OnBody(http_parser* parser, const char* at, size_t length, void* obj) {
    //将obj转换为CHttpParserWrapper*类型，并调用SetBodyContent(at, length)方法，将解析到的消息体内容设置到m_body_content成员变量中
    ((CHttpParserWrapper*)obj)->SetBodyContent(at, length);
    return 0;
}

/**
 * 该函数在解析器完成解析整个消息时进行回调
*/
int CHttpParserWrapper::OnMessageComplete(http_parser* parser, void* obj) {
    //将obj转换为CHttpParserWrapper*类型，并调用SetReadAll()方法，将m_read_all标志设置为 true，表示已读取了全部字段
    ((CHttpParserWrapper*)obj)->SetReadAll();
    return 0;
}


