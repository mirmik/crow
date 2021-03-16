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
void incoming_handler_bind_invoke(crow::packet *pack)
{
	crow::packet_ptr control(pack);
	incoming_handler_bind(control);
}

py::function subscribe_handler_bind;
void subscribe_handler_bind_invoke(crow::packet *pack)
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
		auto buf = self->rawdata();
		return {buf.data(), buf.size()};
	})
	.def("addr", [](packet_ptr & self) -> py::bytes
	{
		auto buf = self->addr();
		return {buf.data(), buf.size()};
	});

	py::class_<node_packet_ptr>(m, "node_packet_ptr", pack)
	.def("message",
	     [](node_packet_ptr & self) -> py::bytes
	{
		auto buf = self.message();
		return {buf.data(), buf.size()};
	});


	py::class_<pubsub_packet_ptr>(m, "pubsub_packet_ptr", pack);

	py::class_<crow::hostaddr>(m, "hostaddr")
	.def(py::init<const crow::hostaddr_view&>())
	.def(py::init<const std::vector<uint8_t>&>())
	.def("view", &crow::hostaddr::view)
	;

	py::class_<crow::hostaddr_view>(m, "hostaddr_view")
	.def(py::init<const crow::hostaddr&>())
	.def(py::init<const std::vector<uint8_t>&>());
	py::implicitly_convertible<crow::hostaddr, crow::hostaddr_view>();

	py::class_<igris::buffer>(m, "igris_buffer")
	.def(py::init<const std::string&>())
	;
	py::implicitly_convertible<std::string, igris::buffer>();
	py::implicitly_convertible<py::str, igris::buffer>();

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

	m.def("send", (crow::packet_ptr(*)(
	                   const crow::hostaddr_view &,
	                   igris::buffer,
	                   uint8_t,
	                   uint8_t,
	                   uint16_t,
	                   bool fastsend))&crow::send);

	m.def("create_udpgate", &crow::create_udpgate);
	m.def("onestep", &crow::onestep);
	m.def("spin", &crow::spin);
	m.def("start_spin", &crow::start_spin);
	m.def("stop_spin", &crow::stop_spin);

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
	           const std::vector<uint8_t> & addr,
	           const std::string & theme,
	           uint8_t ack, uint16_t ackquant,
	           uint8_t rack, uint16_t rackquant)) &subscribe,
	      py::arg("addr"),
	      py::arg("theme"),
	      py::arg("ack"), py::arg("ackquant"),
	      py::arg("rack"), py::arg("rackquant"));

	m.def("publish",
	      (void (*)(
	           const std::vector<uint8_t> & addr,
	           const std::string & theme,
	           const std::string & data,
	           uint8_t ack, uint16_t ackquant)) & publish,
	      py::arg("addr"),
	      py::arg("theme"),
	      py::arg("data"),
	      py::arg("ack"), py::arg("ackquant"));

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
	.def("send", &node::send);

	py::class_<crow::pynode_delegate, crow::node>(m, "PyNode")
	.def(py::init <
	     std::function<void(crow::node_packet_ptr)>,
	     std::function<void(crow::node_packet_ptr)>
	     > ())
	;

	py::class_<crow::msgbox, crow::node>(m, "msgbox")
		.def(py::init<>())
		.def("send", &crow::msgbox::send, ungil(), py::arg("rid"), py::arg("addr"), py::arg("data"), py::arg("qos")=2, py::arg("ackquant")=50, py::arg("fastsend")=false)
		.def("query", &crow::msgbox::query, ungil())
		.def("receive", &crow::msgbox::receive, ungil())
		.def("reply", &crow::msgbox::reply, ungil())
	;

	m.def("fully_empty", &crow::fully_empty);

	m.def("join_spin", &crow::join_spin, py::call_guard<py::gil_scoped_release>());
	m.def("start_spin", &crow::start_spin);
	m.def("stop_spin", &crow::stop_spin);

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