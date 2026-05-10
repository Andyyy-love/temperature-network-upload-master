# temperature-network-upload-master
使用 C 语言在 Linux 下实现一个温度采集上传系统，客户端采集 DS18B20 温度并通过 TCP 上传，服务端使用 poll() 支持多客户端并发接入，双方使用 SQLite 存储数据，并支持客户端断网缓存与恢复补传。
