#ifndef CROW_BROCKER_H
#define CROW_BROCKER_H

#include <stdio.h>
#include <nos/inet/hostaddr.h>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <nos/print.h>
#include <nos/inet/tcp_socket.h>

extern bool brocker_info;
extern bool log_publish;

namespace brocker
{
	class subscriber;

	class theme
	{
		std::set<subscriber*> subs;

	public:
		std::string name;
		size_t count_subscribers() { return subs.size(); }
		void link_subscriber(subscriber* sub) { subs.insert(sub); }
		void unlink_subscriber(subscriber* sub) { subs.erase(sub); }
		void publish(const std::string & data);
	};
	extern std::map<std::string, theme> themes;
	void unlink_theme_subscriber(theme* thm, subscriber* sub);

	//std::set<subscriber*> all_subscribers;

	class subscriber
	{
	public:
		std::set<theme*> thms;
		
		~subscriber() 
		{
			for (auto* thm : thms) 
				unlink_theme_subscriber(thm, this);
		}

		virtual void publish(const std::string & theme, const std::string & data) = 0;
	};

	namespace subscribers
	{
		class crow : public subscriber
		{
		public:
			std::string addr;
			uint8_t qos = 0;
			uint16_t ackquant = 200;

			static std::map<std::string, crow> allsubs;
			static crow * get(const std::string & addr)
			{
				return &allsubs[addr];
			}

			void publish(const std::string & theme, const std::string & data) override;
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

			void publish(const std::string & theme, const std::string & data) override;
		};
	}


	void erase_crow_subscriber(const std::string & addr);
	void erase_tcp_subscriber(const nos::inet::netaddr & addr);

	void publish(const std::string& theme, const std::string& data);
	void crow_subscribe(uint8_t*addr, int alen, const std::string& theme, uint8_t qos, uint16_t ackquant);
	void tcp_subscribe(const std::string& theme, nos::inet::tcp_socket * sock);
}

#endif