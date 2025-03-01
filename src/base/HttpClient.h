/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpClient.h
 Update Time: Tue 13 Jun 2023 11:38:27 CST
 brief: 
*/

#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

#include <string>
#include <curl/curl.h>
#include "public_define.h"

/**
 * CHttpClient 类提供了一些HTTP客户端的功能
 * CHttpClient利用curl库和jsoncpp实现了http客户端Get/Post 和 语音数据的上传下载
*/
class CHttpClient {
public:
    CHttpClient(void);
    ~CHttpClient(void);
public:
    //发送HTTP POST请求，并返回CURLcode类型的结果代码
    CURLcode Post(const string & strUrl, const string & strPost, string & strResponse);
    //发送HTTP GET请求，并返回CURLcode类型的结果代码
    CURLcode Get(const string & strUrl, string & strResponse);
    //上传字节文件到指定URL，并返回服务器的响应字符串
    string UploadByteFile(const string &url, void* data, int data_len);
    //从指定URL下载字节文件，并将下载的数据存储在AudioMsgInfo对象中
    bool DownloadByteFile(const string &url, AudioMsgInfo* pAudioMsg);
};

#endif


