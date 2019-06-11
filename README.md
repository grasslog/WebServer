# High-performance multi-threaded tcp network server

[![](https://img.shields.io/travis/grasslog/WebServer/master.svg)](https://travis-ci.org/grasslog/WebServer)
[![](https://img.shields.io/badge/language-c++-orange.svg)](http://www.cplusplus.com/)
[![](https://img.shields.io/github/license/grasslog/WebServer.svg)](https://github.com/grasslog/WebServer/blob/master/LICENSE)
[![](https://img.shields.io/badge/github.io-@pages-inactive.svg)](https://grasslog.github.io/WebServer/)
[![](https://img.shields.io/badge/blog-@grass-red.svg)](https://blog.csdn.net/qq_42381849)
[![](https://img.shields.io/badge/Gmail-@bookish00grass-important.svg)](https://www.google.com/intl/zh-CN_cn/gmail/about/)

## Introduction

此项目借鉴《muduo网络库》思想，实现了一个网络库轮子web服务器，语言为c++11，并发模型使用Reactor+非阻塞IO+主线程和工作线程的事件循环，思想遵循one loop per thread。可处理静态资源，解析了get、HTTPhead请求，支持HTTP连接，支持管线化请求，并用webBench进行压测。

## Environment

    - OS：CentOS 7
    - complier：g++4.8

## Build

    ./build.sh

## Start server

    ./WebServer [-t thread_numbers] [-p port]

## Example main.cpp

        #include "EventLoop.h"
        #include "Server.h"
        #include <getopt.h>
        #include <string>

        int main(int argc, char *argv[])
        {
            int threadNum = 4;
            int port = 8888;
            std::string logPath = "./WebServer.log";

            int opt;
            const char *str = "t:p:";
            while((opt = getopt(argc, argv, str)) != -1)
            {
                switch(opt)
                {
                    case 't':
                    {
                        threadNum = atoi(optarg);
                        break;
                    }
                    case 'p':
                    {
                        port = atoi(optarg);
                        break;
                    }
                    default: break;
                }
            }

            #ifndef _PTHREADS
                //LOG << "_PTHREADS is not defined !";
            #endif
            EventLoop mainLoop;
            Server myHTTPServer(&mainLoop, threadNum, port);
            myHTTPServer.start();
            mainLoop.loop();
            return 0;
        }

## Technical

- 服务器框架采用Reactor模式，采用epoll边沿触发模式作为IO复用技术作为IO分配器，分发IO事件
- 对于IO密集型请求使用多线程充分利用多核CPU并行处理，创建了线程池避免线程频繁创建销毁的开销
- 主线程只负责accept请求，并以轮回的方式分发给其它IO线程，然后执行read->decode->compute->encode->write
- 使用基于小根堆的定时器关闭超时请求
- 主线程和工作线程各自维持了一个事件循环(eventloop)
- TLS，使用了线程的本地局部存储功能，维护各个线程的运行状态以及运行信息等
- 设计了任务队列的缓冲区机制，避免了请求陷入死锁循环
- 线程间的高效通信，使用eventfd实现了线程的异步唤醒
- 为减少内存泄漏的可能，使用智能指针等RAII机制
- 使用状态机解析了HTTP请求,支持http管线化请求
- 支持优雅关闭连接 

## WebServer demo


![在这里插入图片描述](https://img-blog.csdnimg.cn/20190602114909245.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQyMzgxODQ5,size_16,color_FFFFFF,t_70)

## Code statistics

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190604115528778.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQyMzgxODQ5,size_16,color_FFFFFF,t_70)

## Project imagines

Show more details in CSDN [my blog](https://blog.csdn.net/qq_42381849/article/details/90766452)