/**
	@file
*/

#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <assert.h>
#include <crow/tower.h>
#include <crow/protocol.h>

#include <igris/buffer.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>

#include <string>
#include <vector>

struct crow_theme;

#define SUBSCRIBE 0
#define PUBLISH 1
#define MESSAGE 2

typedef struct crow_subheader_pubsub
{
	uint8_t type;
	uint8_t thmsz;
} __attribute__((packed)) crow_subheader_pubsub_t;

typedef struct crow_subheader_pubsub_data
{
	uint16_t datsz;
} __attribute__((packed)) crow_subheader_pubsub_data_t;

typedef struct crow_subheader_pubsub_control
{
	uint8_t qos;
	uint16_t ackquant;
} __attribute__((packed)) crow_subheader_pubsub_control_t;

namespace crow
{
	class pubsub_protocol_cls : public crow::protocol
	{
	public:
		struct dlist_head themes = DLIST_HEAD_INIT(themes);

		void incoming(crow::packet *pack) override;
		pubsub_protocol_cls() : protocol(CROW_PUBSUB_PROTOCOL) {}

		static void start_resubscribe_thread(int millis);

		void(*incoming_handler)(packet*);

		void resubscribe_all();
	};
	extern pubsub_protocol_cls pubsub_protocol;

	void publish(
	    const uint8_t * raddr, uint8_t rlen,
	    const char *theme,
	    const void* data, uint16_t dsize,
	    uint8_t qos, uint16_t acktime);

	void publish(const uint8_t * raddr, uint8_t rlen,
	             const char *theme,
	             const char *data,
	             uint8_t qos, uint16_t acktime);

	void publish(const uint8_t * raddr, uint8_t rlen,
	             const char* theme,
	             const std::string& data,
	             uint8_t qos, uint16_t acktime);

	void publish(
	    const std::vector<uint8_t> & addr,
	    const std::string & theme,
	    const std::string & data,
	    uint8_t qos,
	    uint16_t acktime);


	//void subscribe(const char *theme,
	//               uint8_t qos = 0, uint16_t acktime = DEFAULT_ACKQUANT,
	//               uint8_t rqos = 0, uint16_t racktime = DEFAULT_ACKQUANT);

	void subscribe(const uint8_t * raddr, uint8_t rlen,
	               const char *theme,
	               uint8_t qo0, uint16_t acktime,
	               uint8_t rqos, uint16_t racktime);

	void subscribe(const std::vector<uint8_t> & addr,
	               const std::string & theme,
	               uint8_t qos, uint16_t acktime,
	               uint8_t rqos, uint16_t racktime);

	void publish_buffer(const char *theme, const void *data, uint16_t datsz,
	                    uint8_t qos, uint16_t acktime);
	void set_publish_host(const uint8_t *hhost, size_t hsize);

	//void set_crowker(const std::string &str);
	std::string envcrowker();
	std::string environment_crowker();

	static inline crow_subheader_pubsub_t *
	get_subheader_pubsub(crow::packet *pack)
	{
		return (crow_subheader_pubsub_t *)pack->dataptr();
	}

	static inline crow_subheader_pubsub_data_t *
	get_subheader_pubsub_data(crow::packet *pack)
	{
		return (
		           crow_subheader_pubsub_data_t *)(pack->dataptr() +
		                   sizeof(crow_subheader_pubsub_t));
	}

	static inline crow_subheader_pubsub_control_t *
	get_subheader_pubsub_control(crow::packet *pack)
	{
		return (
		           crow_subheader_pubsub_control_t *)(pack->dataptr() +
		                   sizeof(crow_subheader_pubsub_t));
	}

	static inline char *packet_pubsub_thmptr(struct crow::packet *pack)
	{
		return pack->dataptr() + sizeof(crow_subheader_pubsub_t) +
		       sizeof(crow_subheader_pubsub_data_t);
	}

	static inline char *packet_pubsub_datptr(struct crow::packet *pack)
	{
		return crow::packet_pubsub_thmptr(pack) +
		       get_subheader_pubsub(pack)->thmsz;
	}

	namespace pubsub
	{
		static inline igris::buffer get_theme(crow::packet *pack)
		{
			assert(pack->header.f.type == CROW_PUBSUB_PROTOCOL);

			struct crow_subheader_pubsub *shps = get_subheader_pubsub(pack);
			// struct crow_subheader_pubsub_data * shps_d =
			// get_subheader_pubsub_data(pack);

			return igris::buffer(crow::packet_pubsub_thmptr(pack), shps->thmsz);
		}

		static inline igris::buffer get_data(crow::packet *pack)
		{
			assert(pack->header.f.type == CROW_PUBSUB_PROTOCOL);

			// struct crow_subheader_pubsub * shps = get_subheader_pubsub(pack);
			struct crow_subheader_pubsub_data *shps_d =
			    get_subheader_pubsub_data(pack);

			return igris::buffer(crow::packet_pubsub_datptr(pack),
			                     shps_d->datsz);
		}
	} // namespace pubsub

	class subscriber
	{
	public:
		dlist_head lnk;

	public:
		const uint8_t * addr;
		uint8_t alen;
		const char * theme;
		uint8_t qos;
		uint16_t ackquant;
		uint8_t rqos;
		uint16_t rackquant;

		igris::delegate<void, crow::packet*> dlg;

	public:
		subscriber() = default;

		void subscribe(
		    const uint8_t * addr,
		    uint8_t alen,
		    const char * theme,
		    uint8_t qos,
		    uint16_t ackquant,
		    uint8_t rqos,
		    uint16_t rackquant,
		    igris::delegate<void, crow::packet*> dlg
		)
		{
			this->addr = addr;
			this->alen = alen;
			this->theme = theme;
			this->qos = qos;
			this->ackquant = ackquant;
			this->rqos = rqos;
			this->rackquant = rackquant;
			this->dlg = dlg;

			system_lock();
			dlist_add(&lnk, &pubsub_protocol.themes);
			system_unlock();

			resubscribe();
		}

		void subscribe(
		    const std::vector<uint8_t>& addr,
		    const char * theme,
		    uint8_t qos,
		    uint16_t ackquant,
		    uint8_t rqos,
		    uint16_t rackquant,
		    igris::delegate<void, crow::packet*> dlg
		) 
		{
			subscribe(addr.data(), addr.size(), theme, qos, ackquant, rqos, rackquant, dlg);
		}

		void resubscribe() 
		{
			crow::subscribe(addr, alen, theme, qos, ackquant, rqos, rackquant);
		}
		
	};
} // namespace crow

#endif