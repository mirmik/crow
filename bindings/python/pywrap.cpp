#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <crow/brocker/crowker.h>

#include <crow/gates/udpgate.h>
#include <crow/packet_ptr.h>
#include <crow/tower.h>
#include <crow/iter.h>
#include <crow/pubsub/pubsub.h>
#include <crow/proto/node.h>
#include "pynode.h"
#include <crow/proto/msgbox.h>
#include <crow/address.h>

using namespace crow;
namespace py = pybind11;
using ungil = py::call_guard<py::gil_scoped_release>;

void register_subscriber_class(py::module & m);

py::function incoming_handler_bind;
void incoming_handler_bind_invoke(crow_packet *pack)
{
	crow::packet_ptr control(pack);
	incoming_handler_bind(control);
}

py::function subscribe_handler_bind;
void subscribe_handler_bind_invoke(crow_packet *pack)
{
	crow::packet_ptr control(pack);
	subscribe_handler_bind(control);
}

PYBIND11_MODULE(libcrow, m)
{
	auto pack = py::class_<packet_ptr>(m, "packet_ptr")
	            .def("rawdata",
	                 [](packet_ptr & self) -> py::bytes
	{
		igris::buffer buf = { 
			crow_packet_dataptr(self.get()), 
			crow_packet_datasize(self.get()) 
		};
		return {buf.data(), buf.size()};
	})
	.def("addr", [](packet_ptr & self) -> crow::hostaddr
	{
		return { 
			crow_packet_addrptr(self.get()), 
			crow_packet_addrsize(self.get())
		};
	});

	py::class_<node_packet_ptr>(m, "node_packet_ptr", pack)
	.def("message",
	     [](node_packet_ptr & self) -> py::bytes
	{
		auto buf = self.message();
		return {buf.data(), buf.size()};
	});


	py::class_<pubsub_packet_ptr>(m, "pubsub_packet_ptr", pack)
	.def("message",
	     [](pubsub_packet_ptr & self) -> py::bytes
	{
		auto buf = self.message();
		return {buf.data(), buf.size()};
	});;

	py::class_<crow::hostaddr>(m, "hostaddr")
	.def(py::init<const crow::hostaddr_view&>())
	.def(py::init<const std::vector<uint8_t>&>())
	.def("view", &crow::hostaddr::view)
	;

	py::class_<crow::hostaddr_view>(m, "hostaddr_view")
	.def(py::init<const crow::hostaddr&>())
	.def(py::init<const std::vector<uint8_t>&>());
	py::implicitly_convertible<crow::hostaddr, crow::hostaddr_view>();

	/*py::class_<std::string_view>(m, "igris_buffer")
	.def(py::init<const std::string&>())
	.def("__init__", [](const py::str & s) -> std::string_view 
	{
        char *buffer;
        ssize_t length;

        PYBIND11_BYTES_AS_STRING_AND_SIZE(s.ptr(), &buffer, &length);

        return {buffer, (size_t) length};
	})
	;
	py::implicitly_convertible<std::string, std::string_view>();
	py::implicitly_convertible<py::str, std::string_view>();*/

	auto  __gateway__ =
	    py::class_<gateway>(m, "gateway")
	    .def("bind", &gateway::bind)
	    .def("finish", &gateway::finish)
	    ;

	py::class_<udpgate>(m, "udpgate", __gateway__)
	.def(py::init<>())
	.def(py::init<uint16_t>())
	.def("open", &udpgate::open)
	.def("close", &udpgate::close)
	;

	m.def("send", [](const crow::hostaddr_view & addr,
	               const std::string& data,
	                   uint8_t type,
	                   uint8_t qos,
	                   uint16_t ackquant,
	                   bool fastsend)
	{
		return crow::send(addr, data, type, qos, ackquant);
	});

	m.def("create_udpgate", &crow::create_udpgate, py::arg("id")=CROW_UDPGATE_NO, py::arg("port")=0);
	m.def("crowker_address", &crow::crowker_address);


	m.def("set_incoming_handler", [](py::function f)
	{
		incoming_handler_bind = f;
		crow::user_incoming_handler = incoming_handler_bind_invoke;
	});

	m.def("set_subscribe_handler", [](py::function f)
	{
		subscribe_handler_bind = f;
		crow::pubsub_protocol.incoming_handler = subscribe_handler_bind_invoke;
	});

	m.def("address", &address);

	m.def("subscribe",
	      (void (*)(
	           const crow::hostaddr & addr,
	           const std::string & theme,
	           uint8_t ack, uint16_t ackquant,
	           uint8_t rack, uint16_t rackquant)) &subscribe,
	      py::arg("addr"),
	      py::arg("theme"),
	      py::arg("ack")=2, py::arg("ackquant")=50,
	      py::arg("rack")=2, py::arg("rackquant")=50);

	m.def("publish",
	      [](
	           const crow::hostaddr & addr,
	           const std::string & theme,
	           const std::string & data,
	           uint8_t ack, uint16_t ackquant) 
	      {
	      	return crow::publish(addr, theme, data, ack, ackquant);
	      },
	      py::arg("addr"),
	      py::arg("theme"),
	      py::arg("data"),
	      py::arg("ack")=2, 
	      py::arg("ackquant")=50);

	static int unused; // the capsule needs something to reference
	py::capsule cleanup(&unused, [](void *)
	{
		user_incoming_handler = nullptr;
		incoming_handler_bind.release();

		crow::pubsub_protocol.incoming_handler = nullptr;
		subscribe_handler_bind.release();
	});
	m.add_object("_cleanup", cleanup);

	m.def("diagnostic_setup", diagnostic_setup);
	m.def("finish", finish);

	py::class_<crow::node>(m, "node")
	.def("bind", py::overload_cast<>(&crow::node::bind))
	.def("bind", py::overload_cast<int>(&crow::node::bind))
	.def_readwrite("id", &node::id)
	.def("send", [](crow::node & node, 
		int rid,
		const crow::hostaddr_view & addr,
	    const std::string& data,
	    uint8_t qos,
	    uint16_t ackquant,
	    bool fastsend)
	{
		return node.send(rid,addr, data, qos, ackquant);
	}, ungil(), py::arg("rid"), py::arg("addr"), py::arg("data"), py::arg("qos")=2, py::arg("ackquant")=50, py::arg("fastsend")=false);

	py::class_<crow::pynode_delegate, crow::node>(m, "PyNode")
	.def(py::init <
	     std::function<void(crow::node_packet_ptr)>,
	     std::function<void(crow::node_packet_ptr)>
	     > ())
	;

	py::class_<crow::msgbox, crow::node>(m, "msgbox")
		.def(py::init<>())
		.def("query", &crow::msgbox::query, ungil())
		.def("receive", &crow::msgbox::receive, ungil())
		.def("reply", &crow::msgbox::reply, ungil())
	;

	m.def("fully_empty", &crow::fully_empty);

	m.def("join_spin", &crow::join_spin, py::call_guard<py::gil_scoped_release>());
	m.def("start_spin", &crow::start_spin);
	m.def("stop_spin", &crow::stop_spin, py::arg("wait")=true);
	m.def("onestep", &crow::onestep);
	m.def("spin", &crow::spin);
	m.def("start_resubscribe_thread", &crow::start_resubscribe_thread);

	m.def("get_gateway", &crow::get_gateway);

	py::class_<crow::crowker>(m, "crowker")
	.def("instance", &crow::crowker::instance, py::return_value_policy::reference)
	.def("publish", &crow::crowker::publish)
	.def("set_info_mode", &crow::crowker::set_info_mode)
	;


	register_subscriber_class(m);
	m.def("enable_crowker_subsystem", &crow::pubsub_protocol_cls::enable_crowker_subsystem);
	m.def("gates", &crow::gates);
	m.def("nodes", &crow::nodes);
}