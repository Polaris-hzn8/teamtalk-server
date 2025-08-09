/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpClient.h
 Update Time: Tue 13 Jun 2023 11:38:27 CST
 brief: 
*/

#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include <curl/curl.h>
#include "public_define.h"

class CHttpClient
{
public:
    CHttpClient(void);
    ~CHttpClient(void);
public:
    CURLcode Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
    CURLcode Get(const std::string & strUrl, std::string & strResponse);
    // 语音数据上传下载
    std::string UploadByteFile(const std::string &url, void* data, int data_len);
    bool DownloadByteFile(const std::string &url, AudioMsgInfo* pAudioMsg);
};

#endif // _HTTP_CLIENT_H_
