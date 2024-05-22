# readme

---

TeamTalk整套服务提供模块部署脚本和一键部署方案，主要模块有NGINX，PHP，MYSQL，REDIS，IM_WEB，IM_SERVER，其中IM_WEB，IM_SERVER为自主开发模块，其余模块均为开源解决方案，各个模块需要手动改动的地方如下：

### IM_SERVER

IM_SERVER下共有8种服务器,所以也需要对这些服务器进行分别配置

	1.LOGIN_SERVER: 
	ClientListenIP为提供给Client端监听的本地地址，走的是pb协议
	HttpListenIP为提供给Client端监听的http协议本地地址(功能与ClientListenIP一样)
	MsgServerListenIP为提供给Msg Server端监听的本地地址
	msfs为提供客户端上传下载图片使用的msfs地址
	discovery为mac,ios,android客户端中“发现”所使用的地址,此处地址填写的是php后台的地址
	
	2.MSG_SERVER: 
	ListenIP为本机监听的IP,用于Client端的消息收发; 
	DBServerIP用于链接DB_PROXY,此处至少填两个数据库地址,也可以是同一个实例
	LoginServerIP用于链接LoginServer
	RouteServerIP用于链接RouteServer
	FileServerIP用于链接FileServer
	PushServerIP用于链接PushServer
	IpAddr填写的是Client端可以直接访问的地址,对于需要公网访问的情况下,如果是路由器映射,则需要填路由器映射在公网上的地址;此处需要填写两个Client端可以访问的地址,如果只有一个,则填写相同的地址即可
	aesKey为msg_server用于消息解密使用,与各个客户端保持一致
	
	3.ROUTE_SERVER: 
	根据说明配置需要监听的对应IP Port即可
	
	4.FILE_SERVER: 
	Address为Client端可以直接可以访问的IP地址,对于需要公网访问的情况下,如果是路由器映射,则需要填路由器映射在公网上的地址
	
	5.MSFS_SERVER: 
	ListenIP和Port填写的是监听的本地IP, BaseDir为默认保存图片文件的路径,如有必要可以更改
	
	6.DB_PROXY_SERVER	
	在安装配置脚本setup.sh中, DB_PROXY的默认监听Port设置为10600,如果被更改需要同时对IM_SERVER中其他服务器的配置进行更改，详见MSG_SERVER, HTTP_MSG_SERVER配置说明;
	对于mariadb与redis的配置可以根据自己的mysql,redis进行设置
	
	7.HTTP_MSG_SERVER
	ListenIP为本地监听的地址，供php后台使用
	DBServerIP用于链接DB_PROXY,此处至少填两个数据库地址,也可以是同一个实例
	RouteServerIP用于链接RouteServer
	
	8.PUSH_SERVER
	ListenIP为本地监听的地址
	CertPath,KeyPath,KeyPassword,SandBox为苹果apns服务所需要参数	

在安装之前可以先执行`setup.sh check`命令进行安装环境，检查通过后对各个模块进行一些配置文件的设置，其中主要设置的为IM_SERVER中的几个服务器地址设置,设置完成后运行`setup.sh install`，

各模块支持安装到不同的服务器上，所以部署可以根据自己的需要进行模块安装，主要修改的地方即为上述各个模块中的IP地址设置。根据自己的网络拓扑在conf文件夹下的各个配置文件中预先设置正确的IP地址，然后执行模块内的setup install即可。













