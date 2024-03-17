/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpClient.cpp
 Update Time: Tue 13 Jun 2023 11:38:33 CST
 brief:
*/

#include "HttpClient.h"
#include "util.h"
#include "json/json.h"
#include <string>
using namespace std;

/**
 * @brief 用于CURL库回调的写入数据
 * 在使用CURL库进行数据接收时，可以通过设置回调函数为 write_data_string，将接收到的数据保存到 string 对象中
 * @param ptr 指向接收到的数据的指针
 * @param size 每个数据块的大小
 * @param nmemb 数据块的数量
 * @param userp 用户指针，通常用于传递额外的数据或对象
 * @return 
*/ 
size_t write_data_string(void* ptr, size_t size, size_t nmemb, void* userp) {
    size_t len = size * nmemb;
    //函数将接收到的数据拼接到 userp 指向的 string 对象中，并返回已写入的数据大小 len
    string* response = (string*)userp;
    response->append((char*)ptr, len);
    return len;
}

/**
 * @brief 用于 CURL 库回调的写入数据函数
 * 通过设置回调函数为 write_data_binary，将接收到的二进制数据保存到指定的数据结构中 如 AudioMsgInfo 对象
 * @param ptr 指向接收到的数据的指针
 * @param size 每个数据块的大小
 * @param nmemb 数据块的数量
 * @param pAudio 指向 AudioMsgInfo 对象的指针，该对象用于存储音频消息的相关信息
 * @return 
*/
size_t write_data_binary(void* ptr, size_t size, size_t nmemb, AudioMsgInfo* pAudio) {
    size_t nLen = size * nmemb;
    //检查是否超过了预设的文件大小限制，避免数据溢出
    if (pAudio->data_len + nLen <= pAudio->fileSize + 4) {
        //函数将接收到的二进制数据拷贝到 pAudio 对象的 data 缓冲区中
        memcpy(pAudio->data + pAudio->data_len, ptr, nLen);
        //并更新 pAudio 对象的 data_len 字段以跟踪已接收的数据长度
        pAudio->data_len += nLen;
    }
    return nLen;
}


CHttpClient::CHttpClient(void) { }

CHttpClient::~CHttpClient(void) { }

/// @brief 静态函数用作 CURL 库的回调函数 用于接收数据并将其保存到字符串对象中
/// @param buffer 指向接收到的数据的指针
/// @param size 每个数据块的大小
/// @param nmemb 数据块的数量
/// @param lpVoid 指向 string 对象的指针，用于保存接收到的数据
/// @return 
static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid) {
    //将lpVoid强制转换为string*类型
    string* str = dynamic_cast<string*>((string*)lpVoid);
    if (NULL == str || NULL == buffer) return -1;

    //将buffer中的数据以 size*nmemb 的长度追加到 string 对象中
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);

    //返回 nmemb 表示成功接收到的数据块数量
    return nmemb;
}

/// @brief Post 函数实现，用于发送 POST 请求并接收响应数据
/// @param strUrl POST 请求的 URL
/// @param strPost POST 请求的数据
/// @param strResponse 保存接收到的响应数据的字符串
/// @return 
CURLcode CHttpClient::Post(const string& strUrl, const string& strPost, string& strResponse) {
    CURLcode res;
    //1.初始化一个CURL句柄curl
    CURL* curl = curl_easy_init();
    if (NULL == curl) return CURLE_FAILED_INIT;

    //2.为curl设置各种选项
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());//设置请求的 URL
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);//禁止重用连接
    curl_easy_setopt(curl, CURLOPT_POST, 1);//启用 POST 请求
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());//设置 POST 请求的数据

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);//不使用自定义的读取数据方式
    //不使用自定义的读取数据方式 意味着该 POST 请求不需要发送任何请求体数据，而是只需要接收服务器的响应数据
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);//设置写数据回调函数为 OnWriteData
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);//设置写数据回调函数的用户数据为 strResponse

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);//禁止使用信号进行超时处理
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);//设置连接超时时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);//设置请求超时时间

    //3.调用curl_easy_perform执行请求，并将返回的结果保存在 res 变量中
    res = curl_easy_perform(curl);

    //4.使用curl_easy_cleanup清理 curl 句柄
    curl_easy_cleanup(curl);

    //5.返回结果 res
    return res;
}

/// @brief 使用libcurl库执行 HTTP GET 请求
/// @param strUrl 要发送HTTP请求的URL
/// @param strResponse 保存HTTP响应的内容
/// @return 
CURLcode CHttpClient::Get(const string& strUrl, string& strResponse) {
    // 1.CURLcode这是一个枚举类型，表示 CURL 库的操作结果
    CURLcode res;

    // 2.初始化 libcurl 库，并返回一个指向 CURL 对象的指针
    CURL* curl = curl_easy_init();
    if (NULL == curl) return CURLE_FAILED_INIT;

    // 3.设置 CURL 对象的选项
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());//设置请求的 URL
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);//指定读取数据的回调函数 NULL，表示不使用读取回调函数
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);//禁用连接的重用 确保每次请求都使用新的连接

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);//指定写入数据的回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);//指定写入回调函数中的用户数据
    /**
     * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作
     * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出
     */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);//在多线程环境中避免使用信号处理函数
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);//设置连接超时时间 s秒
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);//设置请求超时时间 s秒

    //4.调用curl_easy_perform执行请求，并将返回的结果保存在 res 变量中
    res = curl_easy_perform(curl);

    //5.使用curl_easy_cleanup清理 curl 句柄
    curl_easy_cleanup(curl);

    //6.返回结果 res
    return res;
}

/// @brief 将字节数据上传到指定的 URL 地址
/// @param strUrl 要发送 GET 请求的 URL 地址 
/// @param pData 指向要上传的二进制数据的指针
/// @param nSize 要上传的二进制数据的大小 
/// @return String 返回上传成功后的URL
string CHttpClient::UploadByteFile(const string& strUrl, void* pData, int nSize) {
    // strUrl为空直接返回空字符串
    if (strUrl.empty()) return "";

    // 1.初始化一个CURL对象
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    // 2.创建一个curl_slist结构体指针headerlist，用于存储 HTTP 请求的头部信息
    struct curl_slist* headerlist = NULL;
    // 添加了一个头部信息
    headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data; boundary=WebKitFormBoundary8riBH6S4ZsoT69so");

    // 3.设置 CURL 对象的选项
    // what URL that receives this POST
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);//上述头部信息 headerlist 设置给CURL对象
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());//指定要发送请求的 URL
    //curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // enable verbose for easier tracing

    // 4.构建请求体body，包括边界标识和文件数据
    string body = "--WebKitFormBoundary8riBH6S4ZsoT69so\r\nContent-Disposition: form-data; name=\"file\"; filename=\"1.audio\"\r\nContent-Type:image/jpg\r\n\r\n";
    // 这里的请求体是一个多部分表单数据，其中包含一个名为 "file" 的表单字段，其值为字节数据
    // 在请求体的最后添加了一个结束标识。
    body.append((char*)pData, nSize); // image buffer
    string str = "\r\n--WebKitFormBoundary8riBH6S4ZsoT69so--\r\n\r\n";
    body.append(str.c_str(), str.size());

    // post binary data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());//将请求体数据设置给 CURL 对象
    // set the size of the postfields data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());//指定请求体数据的大小

    // 5.创建一个空字符串 strResp，用于接收服务器的响应数据
    string strResp;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_string);//指定一个写入回调函数 write_data_string
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResp);//将strResp的地址传递给写入回调函数，以便在回调函数中操作strResp
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);//禁用进度条显示

    // 6.用curl_easy_perform函数执行HTTP请求，并将返回结果保存在res变量中
    // Perform the request, res will get the return code
    CURLcode res = curl_easy_perform(curl);

    // 7.清理并释放CURL对象
    curl_easy_cleanup(curl);

    // 8.检查请求是否成功，如果res不等于CURLE_OK，则输出错误日志并返回空字符串
    if (CURLE_OK != res) {
        log_error("curl_easy_perform failed, res=%d", res);
        return "";
    }

    // 9.使用Json::Reader解析strResp字符串为 Json::Value对象，以便进一步处理服务器返回的JSON 数据
    // upload 返回的json格式不一样，要特殊处理
    Json::Reader reader;
    Json::Value value;
    // 9-1.检查解析结果，如果解析失败或者返回的JSON中没有error_code字段，则输出错误日志并返回空字符串
    if (!reader.parse(strResp, value)) {
        log_error("json parse failed: %s", strResp.c_str());
        return "";
    }
    if (value["error_code"].isNull()) {
        log_error("no code in response %s", strResp.c_str());
        return "";
    }

    // 9-2.检查返回的错误码 nRet，如果不等于0 则输出错误日志并返回空字符串
    uint32_t nRet = value["error_code"].asUInt();
    if (nRet != 0) {
        log_error("upload faile:%u", nRet);
        return "";
    }

    // 9-3.如果以上条件都满足说明上传成功，返回从JSON中提取的URL
    return value["url"].asString();
}

/// @brief 用于从指定的URL下载字节文件
/// @param url 要下载的文件的URL地址
/// @param pAudioMsg 指向AudioMsgInfo结构体的指针，用于存储下载的字节数据
/// @return 
bool CHttpClient::DownloadByteFile(const string& url, AudioMsgInfo* pAudioMsg) {
    // 1.初始化libcurl
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    // 2.设置curl对象
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());//设置下载的URL地址
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);//设置超时时间
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);//禁止使用信号处理
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);//禁止重用连接

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_binary);//设置回调函数，用于处理下载的字节数据
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pAudioMsg);//设置回调函数的用户参数，用于存储下载的字节数据

    // 3.执行下载操作
    CURLcode res = curl_easy_perform(curl);

    // 4.获取下载的HTTP响应状态码
    int retcode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
    if (CURLE_OK != res || retcode != 200) {
        log_error("curl_easy_perform failed, res=%d, ret=%u", res, retcode);
    }

    // 5.获取下载的文件大小
    double nLen = 0;
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &nLen);

    // 6.清理libcurl资源
    curl_easy_cleanup(curl);

    // 7.验证下载的文件大小是否与预期一致
    if (nLen != pAudioMsg->fileSize) return false;
    return true;
}


