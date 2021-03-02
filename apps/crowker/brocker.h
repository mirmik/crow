#ifndef CROW_BROCKER_H
#define CROW_BROCKER_H

#include <stdio.h>
#include <nos/inet/hostaddr.h>

#include <map>
#include <set>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <nos/print.h>
#include <nos/inet/tcp_socket.h>

extern bool brocker_info;
extern bool log_publish;

namespace brocker
{
	class subscriber;
	struct options {};

	class theme
	{
	public:
		std::set<subscriber*> subs;
		int64_t timestamp_publish;
		int64_t timestamp_activity;

	public:
		std::string name;
		size_t count_subscribers() { return subs.size(); }
		void link_subscriber(subscriber* sub) { subs.insert(sub); }
		bool has_subscriber(subscriber* sub) { return subs.count(sub); }
		void unlink_subscriber(subscriber* sub) { subs.erase(sub); }
		void publish(std::shared_ptr<std::string> data);
	};
	extern std::map<std::string, theme> themes;

	void unlink_theme_subscriber(theme* thm, subscriber* sub);

	class subscriber
	{
		struct themenote 
		{
			options* opts = nullptr;
			~themenote(){ delete opts; }
		};

	public:
		std::unordered_map<theme*, themenote> thms;
		
		~subscriber() 
		{
			for (auto& thm : thms) 
				unlink_theme_subscriber(thm.first, this);
		}

		virtual void publish(const std::string & theme, const std::string & data, options * opts) = 0;
	};

	namespace subscribers
	{
		class crow : public subscriber
		{
		public:
			std::string addr;

			static std::map<std::string, crow> allsubs;
			static crow * get(const std::string & addr)
			{
				return &allsubs[addr];
			}

			void publish(const std::string & theme, const std::string & data, options * opts) override;
		};

		struct crow_options : public options 
		{
			uint8_t qos = 0;
			uint16_t ackquant = 200;
			crow_options(uint8_t qos, uint16_t ackquant) : qos(qos), ackquant(ackquant) {}
		};

		class tcp : public subscriber
		{
		public:
			nos::inet::tcp_socket * sock;

			static std::map<nos::inet::netaddr, tcp> allsubs;
			static tcp * get(const nos::inet::netaddr addr)
			{
				return &allsubs[addr];
			}

			void publish(const std::string & theme, const std::string & data, options * opts) override;
		};
	}


	void erase_crow_subscriber(const std::string & addr);
	void erase_tcp_subscriber(const nos::inet::netaddr & addr);

	void publish(std::shared_ptr<std::string> theme, std::shared_ptr<std::string> data);
	void crow_subscribe(uint8_t*addr, int alen, const std::string& theme, uint8_t qos, uint16_t ackquant);
	void tcp_subscribe(const std::string& theme, nos::inet::tcp_socket * sock);
}

#endif