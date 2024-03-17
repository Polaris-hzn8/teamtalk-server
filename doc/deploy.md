# deploy

---

### 1.第三方依赖

TeamTalk使用了许多第三方库，包括protobuf、hiredis、mariadb(mysql)、log4cxx等等，

服务端对pb,hiredis,mysql_client,log4cxx有依赖，所以服务端需要先安装pb，hiredis,mysql,log4cxx，

可以自行安装或者直接执行目录下的shell脚本自动安装，

- protobuf: sudo ./make_protobuf.sh
- log4cxx: sudo ./make_log4cxx.sh
- [已安装则不用执行]hiredis: make_hiredis.sh
- [已安装则不用执行]mariadb: make_mariadb.sh

这些脚本执行完后会自动将头文件和库文件拷贝至指定的目录

### 2.编译协议文件

所有的协议文件在pb目录下，其中有create.sh以及sync.sh两个shell脚本。

1. create.sh的作用是使用protoc将协议文件转化成相应语言的源码。
2. sync.sh是将生成的源码拷贝到server的目录下。

### 3.编译服务端

经历了编译服务端依赖，pb之后，就可以执行server目录下的build.sh脚本

	一键部署在auto_setup目录下，其中包含了如下几个子目录及文件:
		im_server 与TeamTalk服务端相关的部署，该目录下包含了服务端的配置模板等。
		im_web 与TeamTalk web管理相关的部署，包含了PHP的配置以及php所需nginx相关配置。需要将php目录更名为tt并打包压缩放到此目录下,否则会报如下错误:
		unzip: cannot find or open tt.zip, tt.zip.zip or tt.zip.ZIP。
		mariadb  由于Percona变化太大，所以本次发布我们使用了mariadb作为默认数据库，如果有需要可以根据自己的需求改成mysql，但是请不要在centos 6及以下的系统中使用yum安装的mysql，因为数据库的配置中会设置utf8mb4，以支持emoji表情的存储，而CentOS 6及以下的系统默认使用的是mysql 5.1 默认是不支持utf8mb4的，需要自己编译。
		setup.sh 是部署总入口，会分别调用二级目录下的setup.sh执行安装部署过程。

- sudo ./build_ubuntu.sh version 1.0


使用"./build.sh version 1"编译整个TeamTalk工程,一旦编译完成，会在上级目录生成im_server_x.tar.gz包，该压缩包包含的内容有:

1. sync_lib_for_zip.sh: 将lib目录下的依赖库copy至各个服务器的目录下，启动服务前需要先执行一次该脚本
2. lib: 主要包含各个服务器依赖的第三方库
3. restart.sh: 启动脚本，启动方式为./restart.sh msg_server
4. login_server：
5. msg_server：
6. route_server：
7. db_proxy_server：
8. file_server：
9. push_server：
10. msfs：

### 4.项目配置

#### （1）负载均衡服务

```
loginserver.conf
HttpPort: 对应提供HTTP服务的端口 默认为8080
MsgServerPort: msg服务器上报监听的端口 默认为8100
```

#### （2）消息服务

```
msgserver.conf
ListenPort：连接msg服务器需要的端口
LoginServerIP1：上报msg服务器负载的地址
LoginServerPort1：上报msg服务器负载的端口
RouteServerIP1：route服务器地址和端口
RouteServerPort1：
PushServerIP1：push推送服务器地址和端口
PushServerPort1：
FileServerIP1：文件传输服务器地址和端口
IpAddr1：对外提供的往外ip，需要上报给login 服务器
```

#### （3）路由服务

```
routeserver.conf
ListenIP=0.0.0.0 该服务监听ip
ListenMsgPort=8200 该服务监听端口
```

#### （4）数据库中间件服务

```
dbproxyserver.conf
ListenPort：该服务监听的端口
ThreadNum：线程池线程数量
DBInstances=teamtalk_master,teamtalk_slave配置数据库实例

#teamtalk_master
teamtalk_master_host=127.0.0.1 数据库ip
teamtalk_master_port=3306 数据库端口
teamtalk_master_dbname=teamtalk 数据库名字
teamtalk_master_username=root 用户名
teamtalk_master_password=123456 密码
teamtalk_master_maxconncnt=16 连接池最大连接数量

#未读消息计数器的redis
unread_host=127.0.0.1 redis地址
unread_port=6379 redis端口
unread_db=1 db索引
unread_maxconncnt=16 连接池最大连接数量
```

#### （5）文件传输服务

```
fileserver.conf
ClientListenIP=0.0.0.0 该服务监听的地址
ClientListenPort=8600 该服务监听的端口,供msg server连接
```

#### （6）HTTP reset api服务

```
httpmsgserver.conf
ListenPort=8400 该服务监听
ConcurrentDBConnCnt=4 db_proxy服务连接通道数量
DBServerIP1=127.0.0.1 db_proxy服务地址
DBServerPort1=10600
DBServerIP2=127.0.0.1
DBServerPort2=10600
RouteServerIP1=localhost route路由服务地址
RouteServerPort1=8200
```

#### （7）文件存储服务

```
msfs.conf
ListenIP=0.0.0.0 #可以监听多个IP,用;分割
ListenPort=8700 该服务端口
BaseDir=./tmp 存储地址
FileCnt=0 最大文件数量
FilesPerDir=30000 每个目录最大存储文件
GetThreadCount=32 下载线程
PostThreadCount=1 上传线程
```

#### （8）推送服务

```
pushserver.conf
ListenIP=127.0.0.1
ListenPort=8500 该服务监听端口
CertPath=apns-dev-cert.pem
KeyPath=apns-dev-key.pem
KeyPassword=tt@mogujie
#SandBox
#1: sandbox 0: production
SandBox=0
```

配置地址：https://www.yuque.com/linuxer/xngi03/ol8e7u