# ini_config

---

TeamTalk整套服务提供模块部署脚本和一键部署方案，各个模块需要手动改动的地方如下：

假设所有服务都部署在一台机器上，IM_SERVER下共有8种服务器,所以需要对这些服务器分别进行配置：

在安装之前可以先执行`setup.sh check`命令进行安装环境，检查通过后对各个模块进行配置文件修改，其中主要设置的为IM_SERVER中的几个服务器地址设置,设置完成后运行`setup.sh install`，

各模块支持安装到不同的服务器上，所以部署可以根据自己的需要进行模块安装，主要修改的地方即为上述各个模块中的IP地址设置。根据自己的网络拓扑在conf文件夹下的各个配置文件中预先设置正确的IP地址，然后执行模块内的setup install即可。

### 1.纯内网

安装的机器内网ip为：192.168.1.2

**login_server**：

```ini
ClientListenIP=192.168.1.2						# 提供给客户端监听的本地地址，pb协议
ClientPort=8008
HttpListenIP=192.168.1.2						# 提供给客户端监听的本地地址吗，http协议
HttpPort=8080
MsgServerListenIP=192.168.1.2					# 提供给MsgServer端监听的本地地址
MsgServerPort=8100
msfs=http://192.168.1.2：8700/					# 提供给客户端上传下载图片 使用的msfs地址
discovery=http://192.168.1.2/api/discover		 # mac ios android客户端 "发现" 所使用的地址（此处填写的是php后台的地址）
```

**msg_server**：

```ini
ListenIP=192.168.1.2							# 提供给客户端本地消息收发 本机监听ip地址
ListenPort=8000

ConcurrentDBConnCnt=2							# 连接DB_PROXY 数据库连接地址 至少填两个数据库地址（可以是同个实例）
DBServerIP1=192.168.1.2
DBServerPort1=10600
DBServerIP2=192.168.1.2
DBServerPort2=10600

LoginServerIP1=192.168.1.2						# LoginServer
LoginServerPort1=8100

RouteServerIP1=192.168.1.2						# RouteServer
RouteServerPort1=8400

PushServerIP1=192.168.1.2						# PushServer
PushServerPort1=8500

FileServerIP1=192.168.1.2						# FileServer
FileServerPort1=8600

IpAddr1=192.168.1.2 							# 客户端可以直接访问的地址（如果只有一个填写相同即可）
IpAddr2=192.168.1.2		
MaxConnCnt=100000								# 最大连接数量
aesKey=sdfdsafs									# 消息加解密使用
```

**route_server**：

```ini
ListenIP=192.168.1.2							# 监听的ip地址
ListenMsgPort=8400								# 监听的port端口
```

**msfs_server**：

```ini
ListenIP=192.168.1.2							# 监听的ip地址
ListenPort=8600
BaseDir=./tmp									# 默认保存图片文件的路径
FileCnt=0										# 文件数量
FilesPerDir=30000								# 每个目录下文件数量
GetThreadCount=32								# 下载文件线程数量
PostThreadCount=1								# 上传文件线程数量
```

**file_server**：

```ini
Address=192.168.1.2								# 监听的ip地址
ListenPort=8500
TaskTimeout=60									# 任务超时时间
```

**db_proxy**：

在安装配置脚本setup.sh中，DB_PROXY默认监听端口设置为10600，如果被更改需要同时对IM_SERVER中其他服务器的配置进行更改，详见MSG_SERVER：

```ini
ListenIP=192.168.1.2							# 数据库代理服务器监听ip地址
ListenPort=10600
ThreadNum=48									# double the number of CPU core
MsfsSite=192.168.1.2							# 图片服务器地址

#configure for mysql
DBInstances=teamtalk_master,teamtalk_slave
#teamtalk_master
teamtalk_master_host=127.0.0.1
teamtalk_master_port=3306
teamtalk_master_dbname=teamtalk
teamtalk_master_username=root
teamtalk_master_password=12345
teamtalk_master_maxconncnt=16
#teamtalk_slave
teamtalk_slave_host=127.0.0.1
teamtalk_slave_port=3306
teamtalk_slave_dbname=teamtalk
teamtalk_slave_username=root
teamtalk_slave_password=12345
teamtalk_slave_maxconncnt=16

#configure for unread
CacheInstances=unread,group_set,sync,token,group_member
#未读消息计数器的redis
unread_host=127.0.0.1
unread_port=6379
unread_db=1
unread_maxconncnt=16
#群组消息的redis
group_set_host=127.0.0.1
group_set_port=6379
group_set_db=2
group_set_maxconncnt=16
#同步相关的redis
sync_host=127.0.0.1
sync_port=6379
sync_db=3
sync_maxconncnt=1
#设备标识符的redis
token_host=127.0.0.1
token_port=6379
token_db=4
token_maxconncnt=16
#群组成员的redis
group_member_host=127.0.0.1
group_member_port=6379
group_member_db=5
group_member_maxconncnt=48
```

**http_msg_server**：

```ini
ListenIP=192.168.1.2							# 监听的服务器ip地址 提供给php后台使用
ListenPort=8400

ConcurrentDBConnCnt=4							# 连接DB_PROXY_server地址
DBServerIP1=192.168.1.2
DBServerPort1=10600
DBServerIP2=192.168.1.2
DBServerPort2=10600

RouteServerIP1=192.168.1.2						# 连接routeserver
RouteServerPort1=8200
#RouteServerIP2=localhost
#RouteServerPort2=8201
```

**push_server**：

本配置主要是向移动终端推送消息，第三方服务器参数配置，

```ini
ListenIP=192.168.1.2							# 监听的服务器ip地址
ListenPort=8500

CertPath=apns-dev-cert.pem						# 苹果apns推送通知服务
KeyPath=apns-dev-key.pem
KeyPassword=tt@mogujie
SandBox=0
```

### 2.公网ip

安装的机器为多网卡，包含内网网卡和公网网卡

内网ip为：192.168.1.2

公网ip为122.222.222.222

**login_server**：

	ClientListenIP=122.222.222.222		
	ClientPort=8008
	HttpListenIP=122.222.222.222	
	HttpPort=8080
	MsgServerListenIP=192.168.1.2	
	MsgServerPort=8100
	msfs=http://122.222.222.222	:8700/
	discovery=http://122.222.222.222/api/discover

**msg_server**：

	ListenIP=122.222.222.222
	ListenPort=8000
	
	ConcurrentDBConnCnt=2
	DBServerIP1=192.168.1.2
	DBServerPort1=10600
	DBServerIP2=192.168.1.2
	DBServerPort2=10600
	
	LoginServerIP1=192.168.1.2
	LoginServerPort1=8100
	
	RouteServerIP1=192.168.1.2
	RouteServerPort1=8400
	
	PushServerIP1=192.168.1.2
	PushServerPort1=8500
	
	FileServerIP1=192.168.1.2
	FileServerPort1=8600
	
	IpAddr1=122.222.222.222		
	IpAddr2=122.222.222.222		
	MaxConnCnt=100000

**route_server**：

	ListenIP=192.168.1.2			
	ListenMsgPort=8400

**msfs_server**：

	ListenIP=192.168.1.2;122.222.222.222		
	ListenPort=8600
	BaseDir=./tmp
	FileCnt=0
	FilesPerDir=30000
	GetThreadCount=32
	PostThreadCount=1

**file_server**：

	Address=122.222.222.222	
	ListenPort=8500			
	TaskTimeout=60        

**db_proxy**：

	ListenIP=192.168.1.2	
	ListenPort=10600
	ThreadNum=48		# double the number of CPU core
	MsfsSite=192.168.1.2	
	
	#configure for mysql
	DBInstances=teamtalk_master,teamtalk_slave
	#teamtalk_master
	teamtalk_master_host=127.0.0.1
	teamtalk_master_port=3306
	teamtalk_master_dbname=teamtalk
	teamtalk_master_username=root
	teamtalk_master_password=12345
	teamtalk_master_maxconncnt=16
	
	#teamtalk_slave
	teamtalk_slave_host=127.0.0.1
	teamtalk_slave_port=3306
	teamtalk_slave_dbname=teamtalk
	teamtalk_slave_username=root
	teamtalk_slave_password=12345
	teamtalk_slave_maxconncnt=16


	#configure for unread
	CacheInstances=unread,group_set,token,sync,group_member
	#未读消息计数器的redis
	unread_host=127.0.0.1
	unread_port=6379
	unread_db=1
	unread_maxconncnt=16
	
	#群组设置redis
	group_set_host=127.0.0.1
	group_set_port=6379
	group_set_db=2
	group_set_maxconncnt=16
	
	#同步控制
	sync_host=127.0.0.1
	sync_port=6379
	sync_db=3
	sync_maxconncnt=1
	
	#deviceToken redis
	token_host=127.0.0.1
	token_port=6379
	token_db=4
	token_maxconncnt=16
	
	#GroupMember
	group_member_host=127.0.0.1
	group_member_port=6370
	group_member_db=5
	group_member_maxconncnt=48

**http_msg_server**：

	ListenIP=192.168.1.2
	ListenPort=8400
	
	ConcurrentDBConnCnt=4
	DBServerIP1=192.168.1.2
	DBServerPort1=10600
	DBServerIP2=192.168.1.2
	DBServerPort2=10600
	
	RouteServerIP1=192.168.1.2
	RouteServerPort1=8200
	#RouteServerIP2=localhost	
	#RouteServerPort2=8201

**push_server**：

	ListenIP=192.168.1.2
	ListenPort=8500
	
	CertPath=apns-dev-cert.pem
	KeyPath=apns-dev-key.pem
	KeyPassword=tt@mogujie
	
	#SandBox
	#1: sandbox 0: production
	SandBox=0	

### 3.公网ip+路由器映射

此种情况请确保在内网下可以访问路由器映射的外网ip，

安装的机器为单网卡，外网由路由器映射，

内网ip为: 192.168.1.2，

路由器映射的公网ip为: 122.222.222.222，

**login_server**：

	ClientListenIP=192.168.1.2		
	ClientPort=8008
	HttpListenIP=192.168.1.2
	HttpPort=8080
	MsgServerListenIP=192.168.1.2	
	MsgServerPort=8100
	msfs=http://122.222.222.222	:8700/
	discovery=http://122.222.222.222/api/discover

**msg_server**：

	ListenIP=192.168.1.2
	ListenPort=8000
	
	ConcurrentDBConnCnt=2
	DBServerIP1=192.168.1.2
	DBServerPort1=10600
	DBServerIP2=192.168.1.2
	DBServerPort2=10600
	
	LoginServerIP1=192.168.1.2
	LoginServerPort1=8100
	
	RouteServerIP1=192.168.1.2
	RouteServerPort1=8400
	
	PushServerIP1=192.168.1.2
	PushServerPort1=8500
	
	FileServerIP1=192.168.1.2
	FileServerPort1=8600
	
	IpAddr1=122.222.222.222	 	
	IpAddr2=122.222.222.222		
	MaxConnCnt=100000

**route_server**：

	ListenIP=192.168.1.2			
	ListenMsgPort=8400

**msfs_server**：

	ListenIP=192.168.1.2		
	ListenPort=8600
	BaseDir=./tmp
	FileCnt=0
	FilesPerDir=30000
	GetThreadCount=32
	PostThreadCount=1

**file_server**：

	Address=122.222.222.222	
	ListenPort=8500			
	TaskTimeout=60        

**db_proxy**：

	ListenIP=192.168.1.2
	ListenPort=10600
	ThreadNum=48			# double the number of CPU core
	MsfsSite=192.168.1.2
	
	#configure for mysql
	DBInstances=teamtalk_master,teamtalk_slave
	#teamtalk_master
	teamtalk_master_host=127.0.0.1
	teamtalk_master_port=3306
	teamtalk_master_dbname=teamtalk
	teamtalk_master_username=root
	teamtalk_master_password=12345
	teamtalk_master_maxconncnt=16
	
	#teamtalk_slave
	teamtalk_slave_host=127.0.0.1
	teamtalk_slave_port=3306
	teamtalk_slave_dbname=teamtalk
	teamtalk_slave_username=root
	teamtalk_slave_password=12345
	teamtalk_slave_maxconncnt=16


	#configure for unread
	CacheInstances=unread,group_set,token,sync,group_member
	#未读消息计数器的redis
	unread_host=127.0.0.1
	unread_port=6379
	unread_db=1
	unread_maxconncnt=16
	
	#群组设置redis
	group_set_host=127.0.0.1
	group_set_port=6379
	group_set_db=2
	group_set_maxconncnt=16
	
	#同步控制
	sync_host=127.0.0.1
	sync_port=6379
	sync_db=3
	sync_maxconncnt=1
	
	#deviceToken redis
	token_host=127.0.0.1
	token_port=6379
	token_db=4
	token_maxconncnt=16
	
	#GroupMember
	group_member_host=127.0.0.1
	group_member_port=6379
	group_member_db=5
	group_member_maxconncnt=48

**http_msg_server**：

	ListenIP=192.168.1.2
	ListenPort=8400
	
	ConcurrentDBConnCnt=4
	DBServerIP1=192.168.1.2
	DBServerPort1=10600
	DBServerIP2=192.168.1.2
	DBServerPort2=10600
	
	RouteServerIP1=192.168.1.2
	RouteServerPort1=8200
	#RouteServerIP2=localhost	
	#RouteServerPort2=8201

**push_server**：

	ListenIP=192.168.1.2
	ListenPort=8500
	
	CertPath=apns-dev-cert.pem
	KeyPath=apns-dev-key.pem
	KeyPassword=tt@mogujie
	
	#SandBox
	#1: sandbox 0: production
	SandBox=0	













