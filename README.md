# StaticWebServer
A high performance static web server in c++11
## Introduction
此项目实现了一个web服务器，语言为c++11，并发模型使用Reactor+非阻塞IO+主线程和工作线程的事件循环，思想遵循one loop per thread。可处理静态资源，解析了get、HTTPhead请求，支持HTTP连接，支持管线化请求，并用webBench进行压测。
## Environment
- OS：CentOS 7
- complier：g++4.8
## Build
./build.sh
## Start server
./WebServer [-t thread_numbers] [-p port]
## Technical
- 使用多线程充分利用多核CPU，创建了线程池避免线程频繁创建销毁的开销
- 使用基于小根堆的定时器关闭超时请求
- 使用Reactor模式，采用epoll边沿触发模式作为IO复用技术，设置非阻塞IO
- 主线程只负责accept请求，并以轮回的方式分发给其它IO线程，然后执行read->decode->compute->encode->write
- 主线程和工作线程各自维持了一个事件循环
- 使用缓冲区机制，避免了死锁并且提高效率
- 使用eventfd实现了线程的异步唤醒
- 为减少内存泄漏的可能，使用智能指针等RAII机制
- 使用状态机解析了HTTP请求,支持管线化
- 支持优雅关闭连接  
