#ifndef CROW_DECLARATION_KEEPER
#define CROW_DECLARATION_KEEPER

namespace crow
{
	template <class T>
	class declaration_keeper
	{
		struct record
		{
			chrono::tstamp lastdecl;
			T priv;
		}

		std::mutex mtx;
		int32_t milli_redeclare_limit;

		using map_t igris::dualmap<std::string, std::string, record>;
		map_t map;

		void insert(const std::string& name, const std::string& addr, 
					const T& val) 
		{
			record rec {std::chrono::now(), val};
			map.insert(std::make_pair(name,addr), rec);
		}

		void serve()
		{
			map_t::iter0 it, eit, next;

			it = map.begin();
			eit = map.end();
			next = it->next();

			for (;it != eit; it = next++) 
			{
				if (now - it.second.val.lastdecl > 8s) 
				{
					m.erase(it);
				}
			}
		}
	};
}

#endif