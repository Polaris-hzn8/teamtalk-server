/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpClient.cpp
 Update Time: Tue 13 Jun 2023 11:38:33 CST
 brief:
*/

#include <string>
#include "HttpClient.h"
#include "json/json.h"
#include "common/util.h"

#define REQUEST_TIMEOUT 3
#define CONNECT_TIMEOUT 3

size_t write_data_string(void* ptr, size_t size, size_t nmemb, void* userp)
{
    size_t len = size * nmemb;
    std::string* response = (std::string*)userp;
    response->append((char*)ptr, len);
    return len;
}

size_t write_data_binary(void* ptr, size_t size, size_t nmemb, AudioMsgInfo* pAudio)
{
    size_t nLen = size * nmemb;
    if (pAudio->data_len + nLen <= pAudio->fileSize + 4) {
        memcpy(pAudio->data + pAudio->data_len, ptr, nLen);
        pAudio->data_len += nLen;
    }
    return nLen;
}

CHttpClient::CHttpClient()
{

}

CHttpClient::~CHttpClient()
{
    
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
    if (NULL == str || NULL == buffer)
        return -1;
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}
// POST
CURLcode CHttpClient::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
        return CURLE_FAILED_INIT;

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);  //连接超时时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_TIMEOUT);         //请求超时时间

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return res;
}

// GET
CURLcode CHttpClient::Get(const std::string& strUrl, std::string& strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
        return CURLE_FAILED_INIT;

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_TIMEOUT);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return res;
}

std::string CHttpClient::UploadByteFile(const std::string& strUrl, void* pData, int nSize)
{
    std::string strRet;
    if (strUrl.empty())
        return strRet;

    CURL* curl = curl_easy_init();
    if (!curl)
        return strRet;

    struct curl_slist* headerlist = NULL;
    headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data; boundary=WebKitFormBoundary8riBH6S4ZsoT69so");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    //curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // enable verbose for easier tracing

    // request body
    std::string body = "--WebKitFormBoundary8riBH6S4ZsoT69so\r\nContent-Disposition: form-data; name=\"file\"; filename=\"1.audio\"\r\nContent-Type:image/jpg\r\n\r\n";
    body.append((char*)pData, nSize); // image buffer
    std::string str = "\r\n--WebKitFormBoundary8riBH6S4ZsoT69so--\r\n\r\n";
    body.append(str.c_str(), str.size());

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());

    std::string strResp;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResp);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);//禁用进度条显示

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (CURLE_OK != res) {
        log_error("curl_easy_perform failed, res=%d", res);
        return strRet;
    }

    Json::Value value;
    Json::Reader reader;
    if (!reader.parse(strResp, value)) {
        log_error("json parse failed: %s", strResp.c_str());
        return strRet;
    }
    if (value["error_code"].isNull()) {
        log_error("no code in response %s", strResp.c_str());
        return strRet;
    }

    // 检查错误码返回
    uint32_t nRet = value["error_code"].asUInt();
    if (nRet != 0) {
        log_error("upload faile:%u", nRet);
        return strRet;
    }
    strRet = value["url"].asString();
    return strRet;
}

bool CHttpClient::DownloadByteFile(const std::string& url, AudioMsgInfo* pAudioMsg)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_binary);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pAudioMsg);

    curl_easy_perform(curl);
    
    int retcode = 0;
    CURLcode ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
    if (CURLE_OK != ret || retcode != 200) {
        log_error("curl_easy_perform failed, ret=%d, retcode=%u", ret, retcode);
    }

    double nLen = 0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &nLen);
    curl_easy_cleanup(curl);

    if (nLen != pAudioMsg->fileSize)
        return false;

    return true;
}

