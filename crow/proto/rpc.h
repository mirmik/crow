#ifndef CROW_RPC_H
#define CROW_RPC_H

#include <crow/proto/node.h>

#include <igris/datastruct/dlist.h>
#include <igris/event/delegate.h>
#include <igris/buffer.h>

#include <string>
#include <unordered_map>

#include <igris/serialize/serialize.h>
#include <igris/result.h>
#include <igris/trent/json.h>

#include <nos/io/string_writer.h>
#include <nos/print.h>

#define CROW_RPC_BINARY_FORMAT 0
#define CROW_RPC_TEXT_FORMAT 1

#define CROW_RPC_ERROR_FUNCTION_NOT_FOUNDED -2
#define CROW_RPC_ERROR_UNRESOLVED_FORMAT -4

template<class T> struct sizeof_helper { constexpr static size_t size = sizeof(T); };
template<> struct sizeof_helper<void> { constexpr static size_t size = 0; };

namespace crow
{
	class remote_function_basic
	{
	public:
		virtual int invoke(int8_t format, igris::buffer data, igris::buffer out) = 0;
		virtual int invoke_text_format(int8_t format, igris::buffer data, std::string& out) = 0;
		virtual size_t outsize() = 0;
	};

	template <class Ret, class ... Args>
	class remote_function : public remote_function_basic
	{
		igris::delegate<Ret, Args...> dlg;

	public:
		remote_function(igris::delegate<Ret, Args...> dlg) : dlg(dlg)
		{}

		int invoke(int8_t format, igris::buffer data, igris::buffer output) override;
		int invoke_text_format(int8_t format, igris::buffer data, std::string & output) override;
		size_t outsize() override
		{
			dprln("outsize");
			return sizeof_helper<Ret>::size;
		}
	};

	class rpc_node : public crow::node
	{
		std::unordered_map<std::string, remote_function_basic *> rfuncmap;

	public:
		void bind(int node_no = CROW_RPC_NODE_NO)
		{
			node::bind(node_no);
		}

		template <class Ret, class ... Args>
		void add_delegate(const char * name, igris::delegate<Ret, Args...> dlg)
		{
			rfuncmap[name] = new remote_function<Ret, Args...>(dlg);
		}

		void incoming_packet(crow::packet *pack) override;
	};

	class rpc_request_node : public crow::node
	{
	public:
		crow::packet * incpack;

	public:
		template <class Ret, class ... Args>
		void remote_request(hostaddr addr, nid_t rid, const char * fname, Args && ... args)
		{
			std::string args_data;
			igris::archive::binary_string_writer writer(args_data);

			int8_t format = CROW_RPC_BINARY_FORMAT;

			writer.dump(format);

			// дампим имя функции.
			writer.dump(igris::buffer(fname, strlen(fname)));

			// массово запаковываем аргументы c помощью трикса.
			int ___[] = { (writer.dump(args), 0) ... };

			send(rid, addr, args_data, 2, 50);
		}

		void remote_request_text_format(hostaddr addr, nid_t rid, const char * fname, const std::string& args)
		{
			std::string args_data;
			igris::archive::binary_string_writer writer(args_data);

			int8_t format = CROW_RPC_TEXT_FORMAT;

			writer.dump(format);

			// дампим имя функции.
			writer.dump(igris::buffer(fname, strlen(fname)));

			igris::serialize(writer, args);

			send(rid, addr, args_data, 2, 50);
		}

		void incoming_packet(crow::packet *pack) override
		{
			incpack = pack;
			notify_one(0);
		}

		template <class Ret>
		int parse_incoming(Ret & out)
		{
			int8_t status;

			igris::buffer data = crow::node_data(incpack);
			igris::archive::binary_buffer_reader reader(data);

			reader.load(status);

			if (status != 0)
				return status;

			reader.load(out);
			return 0;
		}

		int parse_incoming_text_format(std::string & out)
		{
			int8_t status;

			igris::buffer data = crow::node_data(incpack);
			igris::archive::binary_buffer_reader reader(data);

			reader.load(status);

			if (status != 0)
				return status;

			igris::deserialize(reader, out);
			return 0;
		}
	};

	class rpc_requestor
	{
		crow::hostaddr 	addr;
		crow::nid_t 	rid;

	public:
		rpc_requestor(crow::hostaddr addr, crow::nid_t rid = CROW_RPC_NODE_NO)
			: addr(addr), rid(rid)
		{}

		template <class Ret, class ... Args>
		int request(const char * fname, Ret & out, Args && ... args)
		{
			rpc_request_node wnode;
			int status;

			wnode.bind();

			wnode.remote_request<Ret, Args...>(
			    addr, rid, fname, std::forward<Args>(args) ...);

			wnode.waitevent();

			status = wnode.parse_incoming<Ret>(out);

			crow::release(wnode.incpack);
			return status;
		}

		int request_text_format(const char * fname, std::string& out, const std::string& in)
		{
			rpc_request_node wnode;
			int status;

			wnode.bind();

			wnode.remote_request_text_format(
			    addr, rid, fname, in);

			nos::println("waitevent");
			wnode.waitevent();

			nos::println("parse_incoming_text_format");
			status = wnode.parse_incoming_text_format(out);

			crow::release(wnode.incpack);
			nos::println("release");
			return status;
		}
	};
}


template <class Ret, class ... Args>
int crow::remote_function<Ret, Args...>::invoke(int8_t format, igris::buffer data, igris::buffer out)
{
	std::tuple<Args ...> args = igris::deserialize<std::tuple<Args ...>>(data);

	Ret ret = std::apply(dlg, args);

	igris::archive::binwriter writer(out.data(), out.size());
	writer.dump(ret);

	return 0;
}

template <class Arg>
static void __bind(Arg & arg, const igris::trent& tr)
{
	if constexpr (std::is_floating_point_v<Arg>) 
	{
		arg = tr.as_numer();
	} 
	else if constexpr (std::is_integral_v<Arg>) 
	{
		arg = tr.as_numer();
	}
	else if constexpr (std::is_same_v<Arg, std::string>) 
	{
		arg = tr.as_string();
	}

	nos::println(arg);
}

template <class Tuple, size_t ... I>
static void __expand(std::index_sequence<I...>, Tuple&& tpl, const igris::trent& tr)
{
	std::apply([&](auto && ... args)
	{
		(__bind(args, tr[I]), ...);
	}, tpl);
}


template <class Ret, class ... Args>
int crow::remote_function<Ret, Args...>::invoke_text_format(
    int8_t format, igris::buffer data, std::string & out)
{
	std::string strargs = igris::deserialize<std::string>(data);

	igris::trent tr = igris::json::parse(strargs);

	std::tuple<Args ...> args;
	//trent_to_tuple(args, tr);
	__expand(std::make_index_sequence<sizeof...(Args)>(), args, tr);

	Ret ret = std::apply(dlg, args);
	//nos::println(ret);

	//igris::archive::binary_string_writer writer(out);
	//igris::serialize(writer, ret);
	
	nos::string_writer writer(out);

	nos::print_to(writer, ret);

	return 0;
}

#endif