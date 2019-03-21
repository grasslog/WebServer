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