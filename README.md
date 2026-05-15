# TemperatureNetworkUpload

这是一个基于 C 语言的温度采集上传项目，适用于树莓派/Linux 环境。

客户端读取 DS18B20 温度传感器数据，将设备 ID、采样时间、温度值打包成 JSON 字符串，通过 TCP 上传到服务器。服务器使用 `poll()` 多路复用支持多个客户端连接，收到 JSON 数据后解析并写入 SQLite 数据库。

客户端支持断线重连：网络断开时，采样数据会暂存到本地 SQLite；重新连上服务器后，会自动把缓存数据补传到服务器。

## 目录说明

```text
client/   树莓派客户端程序，负责采样、JSON 打包、TCP 上传、断线缓存
server/   Linux 服务器程序，负责监听端口、接收 JSON、解析并写入数据库
install/  SQLite 头文件、工具和相关依赖文件
```

## 环境依赖

推荐在树莓派或 Linux 系统上编译运行。

需要安装：

```bash
sudo apt update
sudo apt install gcc make sqlite3 libsqlite3-dev
```

如果要真实采集 DS18B20 温度，需要先启用树莓派 1-Wire，并确认设备存在：

```bash
ls /sys/bus/w1/devices/
```

正常情况下应能看到类似：

```text
28-xxxxxxxxxxxx
```

## 编译服务器

进入 `server` 目录：

```bash
cd temperature-network-upload-master/server
make
```

编译成功后会生成：

```text
server/testserver
```

如果需要重新编译：

```bash
make clean
make
```

## 运行服务器

例如监听 `8900` 端口：

```bash
./testserver -p 8900
```

后台运行：

```bash
./testserver -b -p 8900
```

服务器收到客户端数据后，会写入：

```text
server/server.db
```

查看数据库：

```bash
sqlite3 server.db
```

进入 SQLite 后执行：

```sql
select * from temperature;
.quit
```

## 编译客户端

进入 `client` 目录：

```bash
cd temperature-network-upload-master/client
make
```

编译成功后会生成：

```text
client/INSTALL/bin/testclient
```

如果需要重新编译：

```bash
make clean
make
```

## 运行客户端

建议进入可执行文件所在目录运行：

```bash
cd temperature-network-upload-master/client/INSTALL/bin
```

如果服务器和客户端在同一台机器上：

```bash
./testclient -i 127.0.0.1 -p 8900 -t 6
```

如果服务器在另一台机器上，把 `127.0.0.1` 改成服务器 IP：

```bash
./testclient -i 192.168.1.100 -p 8900 -t 6
```

参数说明：

```text
-i  服务器 IP 或域名
-p  服务器端口
-t  采样和上传间隔，单位为秒
```

客户端上传的 JSON 格式示例：

```json
{"id":"RPI@0520","time":"2026-05-10 23:18:55","temperature":23.94}
```

## 断线缓存与补传

如果服务器断开，客户端会打印类似：

```text
server offline, packet saved into sqlite
```

此时数据会写入客户端本地数据库：

```text
client/INSTALL/etc/client.db
```

服务器恢复后，客户端会自动重连，并补传缓存数据：

```text
cached packet reuploaded
```

## 推荐运行顺序

终端 1，启动服务器：

```bash
cd temperature-network-upload-master/server
./testserver -p 8900
```

终端 2，启动客户端：

```bash
cd temperature-network-upload-master/client/INSTALL/bin
./testclient -i 127.0.0.1 -p 8900 -t 6
```

## 主要模块

客户端：

```text
main.c       客户端主流程
socket.c/h   TCP 连接、断线检测、重连
ds18b20.c/h  DS18B20 温度采样
packet.c/h   数据采样打包、JSON 生成
database.c/h SQLite 缓存数据库操作
logger.c/h   日志系统
```

服务器：

```text
socket-poll.c 服务器主流程，使用 poll() 支持多客户端
socket.c/h    TCP 监听 socket 初始化
packet.c/h    JSON 数据解析
cJSON.c/h     JSON 解析接口
database.c/h  SQLite 数据库存储
```
## 内存泄漏检查Valgrind命令
```
cd INSTALL/bin
valgrind --leak-check=full --show-leak-kinds=all ./testclient -i 127.0.0.1 -p 8080 -t 3
```
```
--leak-check=full   详细检查内存泄漏。


--show-leak-kinds=all 把所有类型的泄漏信息都显示出来，包括：
definitely lost    明确泄漏
indirectly lost    间接泄漏
possibly lost      可能泄漏
still reachable    程序退出时还可访问

```

```
正常结束后，如果看到：
==4455== All heap blocks were freed -- no leaks are possible
说明没有内存泄漏。
```
```
Invalid read ... libarmmem-v7l.so
这个可以先不用管，它更像是树莓派优化库和 Valgrind 的兼容问题，不是业务代码泄漏。
```