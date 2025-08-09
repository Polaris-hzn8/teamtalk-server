/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: db_proxy_server.cpp
 Update Time: Wed 14 Jun 2023 23:05:27 CST
 brief:
*/

#include "netlib.h"
#include "EncDec.h"
#include "version.h"
#include "ProxyConn.h"
#include "HttpClient.h"
#include "ConfigFileReader.h"

#include "DBPool.h"
#include "CachePool.h"
#include "SyncCenter.h"
#include "ThreadPool.h"

#include "business/AudioModel.h"
#include "business/FileModel.h"
#include "business/GroupMessageModel.h"
#include "business/GroupModel.h"
#include "business/MessageModel.h"
#include "business/RelationModel.h"
#include "business/SessionModel.h"
#include "business/UserModel.h"

std::string strAudioEnc;
// this callback will be replaced by imconn_callback() in OnConnect()
void proxy_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    if (msg == NETLIB_MSG_CONNECT) {
        CProxyConn* pConn = new CProxyConn();
        pConn->OnConnect(handle);
    } else {
        log("!!!error msg: %d", msg);
    }
}

int main(int argc, char* argv[])
{
    if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
        printf("Server Version: DBProxyServer/%s\n", VERSION);
        printf("Server Build: %s %s\n", __DATE__, __TIME__);
        return 0;
    }

    //忽略SIGPIPE信号 以避免在处理网络连接时收到此信号导致进程退出
    signal(SIGPIPE, SIG_IGN);
    srand(time(NULL));

    // CacheManager初始化 Redis相关
    CacheManager* pCacheManager = CacheManager::getInstance();
    if (!pCacheManager) {
        log("CacheManager init failed");
        return -1;
    }

    // CDBManager初始化 MySQL相关
    CDBManager* pDBManager = CDBManager::getInstance();
    if (!pDBManager) {
        log("DBManager init failed");
        return -1;
    }

    puts("db init success");

    // 初始化各单例对象
    if (!CAudioModel::getInstance())
        return -1;
    if (!CGroupMessageModel::getInstance())
        return -1;
    if (!CGroupModel::getInstance())
        return -1;
    if (!CMessageModel::getInstance())
        return -1;
    if (!CSessionModel::getInstance())
        return -1;
    if (!CRelationModel::getInstance())
        return -1;
    if (!CUserModel::getInstance())
        return -1;
    if (!CFileModel::getInstance())
        return -1;

    // 服务配置读取
    CConfigFileReader config_file("dbproxyserver.conf");
    char* listen_ip = config_file.GetConfigName("ListenIP");            //监听ip地址
    char* str_listen_port = config_file.GetConfigName("ListenPort");    //端口号
    char* str_thread_num = config_file.GetConfigName("ThreadNum");      //线程数量
    char* str_file_site = config_file.GetConfigName("MsfsSite");        //msfs多媒体文件存储服务器地址
    char* str_aes_key = config_file.GetConfigName("aesKey");            //AES密钥
    if (!listen_ip || !str_listen_port || !str_thread_num || !str_file_site || !str_aes_key) {
        log("missing ListenIP/ListenPort/ThreadNum/MsfsSite/aesKey, exit...");
        return -1;
    }

    // 5.配置文件读取的数据进行设置
    // 5-1.对敏感信息进行加密，以保护数据的安全性
    // 检查AES密钥的有效性
    if (strlen(str_aes_key) != 32) {
        log("aes key is invalied");
        return -2;
    }
    string strAesKey(str_aes_key, 32);
    CAes cAes = CAes(strAesKey);
    string strAudio = "[语音]";
    // 加密语音信息：使用AES加密算法对字符串"[语音]"进行加密，生成加密后的字符串strAudioEnc
    char* pAudioEnc;
    uint32_t nOutLen;
    if (cAes.Encrypt(strAudio.c_str(), strAudio.length(), &pAudioEnc, nOutLen) == 0) {
        strAudioEnc.clear();
        strAudioEnc.append(pAudioEnc, nOutLen);
        cAes.Free(pAudioEnc);
    }

    // 5-2.将字符串转换为整数：将监听端口号和线程数量的字符串表示转换为对应的整数值
    uint16_t listen_port = atoi(str_listen_port);//监听端口号
    uint32_t thread_num = atoi(str_thread_num);//线程数量

    // 5-3.设置文件站点URL：将文件站点URL设置到CAudioModel的实例中
    string strFileSite(str_file_site);
    CAudioModel::getInstance()->setUrl(strFileSite);


    // 6.初始化网络库：调用netlib_init()函数初始化网络库 用于支持tcp请求
    int ret = netlib_init();
    if (ret == NETLIB_ERROR) return ret;

    // 7.初始化CURL库：调用curl_global_init()函数初始化CURL库，用于支持HTTP请求
    curl_global_init(CURL_GLOBAL_ALL);

    // 8.初始化代理连接：调用init_proxy_conn()函数初始化代理连接
    init_proxy_conn(thread_num);

    // 9.初始化同步中心：获取CSyncCenter的实例并进行初始化
    CSyncCenter::getInstance()->init();
    CSyncCenter::getInstance()->startSync();//启动同步线程，用于处理同步操作

    // 10.监听多个IP地址和端口
    // 根据配置文件中的ListenIP和ListenPort配置项 使用netlib_listen()函数在每个IP地址和端口上启动监听
    // 并指定回调函数proxy_serv_callback
    CStrExplode listen_ip_list(listen_ip, ';');
    for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
        ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, proxy_serv_callback, NULL);
        if (ret == NETLIB_ERROR) return ret;
    }

    printf("server start listen on: %s:%d\n", listen_ip, listen_port);
    printf("now enter the event loop...\n");
    writePid();

    // 11.调用netlib_eventloop()函数进入事件循环，等待事件的发生
    netlib_eventloop(10);

    return 0;
}
