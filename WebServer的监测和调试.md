# WebServer的监测和调试

## tcpdump
tcpdump是一款经典的网络抓包工具
$sudo tcpdump tcp port 8888 and host 127.0.0.1
监测所有经过主机127.0.0.1端口8888的tcp数据包
## lsof(list open file)
lsof是一个列出当前系统打开的文件描述符的工具。通过它我们可以了解感兴趣的进程打开了哪些文件描述符，或者我们感兴趣的文件描述符被哪些进程打开了。

$ lsof -c WebServer
## nc(netcat)
命令短小精干、功能强大，它主要被用来快速构建网络连接。我们可以让它以服务器的方式运行，监听某个端口并接受客户连接，我们也可以让它以客户端的方式运行，向服务器发送连接并接受数据。

$ nc -C 127.0.0.1 8888
GET http://localhost:/8888/hello HTTP/1.1
Host: localhost

HTTP/1.1 404 Bad request
Content-Length: 49
Connection: close
## strace
starce是测试服务器性能的重要工具。它跟踪程序运行过程中执行的系统调用和接收到的信号，并将系统调用名、参数、返回值及信号名输出到标准输出或指定的文件。

$ strace -p pid
## netstat
netstat是一个强大的网络信息统计工具，他可以打印本地网卡接口上的全部链接、路由表信息、网卡接口信息等等。

$ netstat -nat | grep 127.0.0.1:8888
## vmstat
vmstat是virtual memory statistics的缩写，它能实时输出系统的各种资源的使用情况，比如进程信息、内存使用、CPU使用率以及I/O使用情况。

$ vmstat 5 3
## ifstat
ifstat是interface statistics的缩写，它是一个简单的网络流量监测工具

$ ifstat -a 2 5
## mpstat
mapstat是mutil-processor statictics的缩写，他能实时的监测多处理器系统中每个CPU的使用情况。

$ mpstat -P ALL 5 2
