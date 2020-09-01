#include <crow/proto/node.h>
#include <igris/osinter/wait.h>

int crow::node::waitevent() 
{
	void * ret;
	wait_current_schedee(&waitlnk, 0, &ret);	
	return (uintptr_t)ret;
}

void crow::node::notify_one(int future) 
{
	unwait_one(&waitlnk, (void*)(uintptr_t)future);
}

void crow::node::notify_all(int future) 
{
	unwait_all(&waitlnk, (void*)(uintptr_t)future);	
}
