#ifndef CROW_CROW_H
#define CROW_CROW_H

#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>

#include <thread>

namespace crow
{
	static inline void run_background()
	{
		std::thread thr(crow::spin);
		thr.detach();
		//return std::move(thr);
	}
}

#endif