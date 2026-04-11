# teamtalk-server

TeamTalk 服务端后台，**C++** 实现。整体为 **多进程 IM 类架构**：每个业务角色对应一个可执行进程，进程间通过 **TCP**、自研 **netlib** 与 **Protocol Buffers** 协作；各服务在自身目录下使用 **`.conf`** 配置。

---

## 主要服务进程

| 目录 | 说明 |
|------|------|
| `login_server` | 登录与鉴权 |
| `msg_server` | 客户端长连接枢纽，对接 DB 代理、登录、路由、推送、文件等 |
| `route_server` | 消息路由与转发 |
| `db_proxy_server` | 数据库访问代理（含 Redis 相关逻辑） |
| `file_server` | 文件服务 |
| `msfs` | 多媒体文件服务 |
| `http_msg_server` | HTTP 接入与消息相关能力 |
| `push_server` | 消息推送 |
| `etcd_login_server` | 基于 etcd 的登录/发现（按需构建部署，未包含在默认一键构建列表时请参考该目录） |

默认一键构建与打包脚本中的服务列表见下文 **`build_server.sh`**。

---

## 目录结构（概要）

```
teamtalk-server/
├── build_server.sh         # 编译与打包入口（在仓库根执行；内部自动 cd 到 src/）
├── format_code.py          # Python 格式化入口，使用 Google 风格（.clang-format）格式化自有 C/C++ 代码
├── .clang-format           # clang-format 规则（BasedOnStyle: Google）
├── readme.md
└── src/
    ├── base/               # 公共静态库：netlib、protobuf 生成代码、jsoncpp、线程池、工具等
    ├── third/              # 第三方依赖（protobuf、slog、hiredis、cetcd、log4cxx、libsecurity 等）
    ├── *_server/           # 各业务服务源码与 CMakeLists.txt
    ├── msfs/
    ├── daeml/              # 守护/工具二进制，打包脚本会尝试编译并打入发布包
    ├── build_server.sh     # 兼容入口：转发到上级目录的 build_server.sh
    └── scripts/            # 运行期脚本（如 server_manager、初始化 SQL 等）
```

编译产物一般在各服务目录下的 `build/`、`bin/`（见 `.gitignore`）。

---

## 构建说明

### 环境要求

- **Linux**（脚本与路径按 Linux 习惯编写）
- **CMake**（各子工程 `CMakeLists.txt` 要求较低版本，实际建议使用系统自带的较新 CMake）
- **GCC/G++**，支持 **C++11**
- 需先准备好 **`src/third/`** 下的依赖库与头文件（如 protobuf、slog、log4cxx 等）。仓库中可能仅保留源码包或部分文件，需按团队流程解压/编译；可参考例如：
  - `src/third/make_protobuf.sh`
  - `src/third/make_log4cxx.sh`
  - `src/third/make_hiredis.sh`
  - `src/third/libsecurity/unix/build.sh`

### 使用 `build_server.sh`

在**仓库根目录**执行（脚本会自动进入 `src/` 再编译，路径行为与原先一致）：

```bash
./build_server.sh clean               # 清理各服务 build/bin 及打包临时目录
./build_server.sh version 1.0.0     # 全量编译并打发布包（版本号写入 src/base/version.h）
./build_server.sh pack 1.0.0        # 仅打包当前已编译好的二进制（仍会重建 daeml）
./build_server.sh base              # 只编译 base 静态库
./build_server.sh msg_server        # 只编译某一个服务
./build_server.sh sync login_server # 调试：同步已编译二进制到打包目录并重启（需已存在 im_server_pack）
```

仍可在 `src/` 下执行 `./build_server.sh ...`，该文件会转发到根目录脚本。

脚本会检查各服务（除 `base`）是否存在 **`服务名/服务名.conf`** 以及 `third/slog/log4cxx.properties` 等。全量打包成功后会在 **`src/`** 下生成 **`im-server-<version>.tar.gz`**，在仓库根目录生成 **`im_server_pack/`**。发布包内容结构可参考 `src/package_realease.txt`。

---

## 代码格式化

依赖系统已安装的 **`python3`** 与 **`clang-format`**（或通过环境变量指定：`CLANG_FORMAT=clang-format-15 ./format_code.py`）。

在**仓库根目录**执行：

```bash
./format_code.py
```

默认格式化 `daeml/`、`server/`、`test/` 下的 C/C++ 源文件，并 **跳过任意 `third/` 目录**，避免改动第三方代码。规则由根目录 **`.clang-format`** 指定（Google 风格）。

---

## 其他说明

- 运行时通常需要 **MySQL / Redis** 等（由 `db_proxy_server` 等配置决定），数据库表结构可参考 `src/scripts/init.sql`。
- 线上启停与监控可使用打包目录中的 `server_manager.sh`、`server_monitor.sh`（由 `build_server.sh` 拷贝到发布包）。
