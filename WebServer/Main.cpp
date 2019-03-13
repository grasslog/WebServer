#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"

int main()
{
	#ifdef _PTHREADS
		//LOG << "_PTHREADS is not defined !";
	#endif
	EventLoop mainLoop;
	Server myHTTPServer(&mainLoop, 4, 8888);
	myHTTPServer.start();
	mainLoop.loop();
	return 0;
}