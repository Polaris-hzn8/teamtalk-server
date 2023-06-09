# teamtalk-server

---

本项目为teamtalk的服务端后台，由C++语言开发

目录结构：

1. run：编译生成的二进制可执行文件
2. src：源代码目录
    - base：基础类库
    - db_proxy_server：数据库代理服务器
    - file_server：文件服务器
    - http_msg_server：http连接服务器
    - login_server：登录服务器
    - msfs：多媒体文件服务器
    - msg_server：消息服务器
    - push_server：消息推送服务器
    - route_server：路由服务器
    - libsecurity：数据安全库
    - etcd_login_server：etcd登录服务器
    - cetcd：etcd的面向C++提供的接口
    - hiredis：redis面向C++提供的接口
    - log4cxx：Apache提供的日志库
    - slog：自定义日志库
    - test：项目测试
    - test_db_proxy：数据库代理服务器测试
    - tools：工具

```
server:.
├─run
│  ├─db_proxy_server
│  ├─file_server
│  ├─http_msg_server
│  ├─login_server
│  ├─msfs
│  ├─msg_server
│  ├─push_server
│  └─route_server
└─src
    ├─base
    │  ├─jsoncpp
    │  ├─pb
    │  ├─security
    │  └─slog
    ├─cetcd
    ├─db_proxy_server
    ├─etcd_login_server
    ├─file_server
    ├─hiredis
    ├─http_msg_server
    ├─libsecurity
    │  ├─android
    │  ├─include
    │  ├─ios
    │  ├─lib
    │  ├─src
    │  ├─unix
    │  └─win
    ├─log4cxx
    ├─login_server
    ├─msfs
    ├─msg_server
    ├─protobuf
    ├─push_server
    ├─route_server
    ├─slog
    ├─test
    ├─test_db_proxy
    └─tools
```

