#ifndef CROW_NOS_LOGGER_H
#define CROW_NOS_LOGGER_H

#include <nos/log/logger.h>
#include <nos/timestamp.h>

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
			char buftime[32];
			char buffer0[128];
			char buffer1[128];
			nos::timestamp(buftime, 32);
			nos::format_buffer(buffer0, fmt, arglist);
			nos::format_buffer(buffer1, "[{}|{}] {}", _name, buftime, buffer0);
		}
	};
}

#endif