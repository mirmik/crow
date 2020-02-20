#ifndef CROW_NOS_LOGGER_H
#define CROW_NOS_LOGGER_H

#include <nos/log/logger.h>
#include <nos/timestamp.h>

#include <crow/proto/pubsub.h>

namespace crow 
{
	class publish_logger : public nos::log::logger 
	{
	public:
		uint8_t * addr;
		uint8_t alen;
		const char* theme;

		const char * _name = "crowlog";

		void init(
			uint8_t * addr,
			uint8_t alen,
			const char* theme) 
		{
			this -> addr = addr;
			this -> alen = alen;
			this -> theme = theme;
		}

		void log(nos::log::level lvl, const char* fmt, const nos::visitable_arglist& arglist) 
		{
			DPRINT(fmt);
			char buftime[32];
			char buffer0[128];
			char buffer1[128];
			nos::timestamp(buftime, 32);
			nos::format_buffer(buffer0, fmt, arglist);
			nos::format_buffer(buffer1, "[{}|{}] {}\r\n", _name, buftime, buffer0);
			crow::publish(addr, alen, theme, buffer1, strlen(buffer1), 0, 200);
		}
	};
}

#endif