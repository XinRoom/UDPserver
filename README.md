# 一个基于C的UDP服务应用
    做个记录

## 主要使用以下库
库 | 解释
-- | --
pthread | 多线程库
event2  | 高性能事件通知库
zdb     | 线程安全的连接池数据库API

## 文件结构

```
| - server2.c     main入口，业务实现
| - server2d.c    server2的守护进程
| - sha2.c        sha2哈希加密
| - threadpool.c  线程池、队列  (来源：manmao/Module)
| - UDPserver.c   UDP并发服务核心
```
## 编译命令
debian 9 

`apt install libevent-dev libzdb-dev`

`gcc server2.c UDPserver.c threadpool.c sha2.c -o server2 -I /usr/include/zdb -levent -lpthread -lzdb`