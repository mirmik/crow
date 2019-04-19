#ifndef CROW_HOLDERS_H
#define CROW_HOLDERS_H

#include <crow/packet.h>
#include <crow/pubsub.h>
#include <crow/tower.h>

#include <igris/util/setget.h>

namespace crow
{
	class packref
	{
	  protected:
		crow::packet *pack;

	  public:
		packref(crow::packet *pack_) : pack(pack_) { pack->refs++; }

		packref(const crow::packref &oth) : pack(oth.pack) { pack->refs++; }

		packref(crow::packref &&oth) : pack(oth.pack) { oth.pack = nullptr; }

		~packref()
		{
			if (pack)
			{
				pack->refs--;

				if (pack->refs == 0)
					crow::release(pack);
			}
		}

		VALUE_GETTER(addr, pack->addr());

		VALUE_GETTER(rawdata, pack->rawdata());

		VALUE_GETTER(qos, pack->header.qos);

		VALUE_GETTER(ackquant, pack->header.ackquant);
	};

	class node_packref : public packref
	{
	};

	class pubsub_packref : public packref
	{
	  public:
		pubsub_packref(crow::packet *pack_) : packref(pack_) {}

		igris::buffer theme() { return pubsub::get_theme(pack); }
	};

	class pubsub_data_packref : public pubsub_packref
	{
	  public:
		pubsub_data_packref(crow::packet *pack_) : pubsub_packref(pack_) {}

		igris::buffer data() { return pubsub::get_data(pack); }
	};

	class pubsub_control_packref : public pubsub_packref
	{
	};

} // namespace crow

#endif