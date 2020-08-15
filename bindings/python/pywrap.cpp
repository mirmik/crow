#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <crow/gates/udpgate.h>
//#include <crow/holders.h>
#include <crow/packet_ptr.h>
#include <crow/tower.h>
#include <crow/proto/pubsub.h>
#include <crow/proto/node.h>
#include <crow/proto/pynode.h>
#include <crow/proto/msgbox.h>
#include <crow/address.h>

//#include <igris/print.h>

using namespace crow;
namespace py = pybind11;
using ungil = py::call_guard<py::gil_scoped_release>;

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

	py::class_<packet_pubsub_ptr>(m, "packet_pubsub_ptr", pack)
	.def(py::init<const packet_ptr&>())
	.def("theme", [](packet_pubsub_ptr & self) -> py::bytes
	{
		auto buf = self.theme();
		return {buf.data(), buf.size()};
	})
	.def("data", [](packet_pubsub_ptr & self) -> py::bytes
	{
		auto buf = self.data();
		return {buf.data(), buf.size()};
	})
	;

	/*py::class_<pubsub_packref, packref>(m, "pubsub_packref")
		.def("theme", [](pubsub_packref &self) -> py::str {
			auto buf = self.theme();
			return {buf.data(), buf.size()};
		});

	py::class_<pubsub_data_packref, pubsub_packref>(m, "pubsub_data_packref")
		.def("data", [](pubsub_data_packref &self) -> py::bytes {
			auto buf = self.data();
			return {buf.data(), buf.size()};
		});*/


	py::class_<crow::hostaddr>(m, "crow_hostaddr")
	.def(py::init<const std::vector<uint8_t>&>())
	;


	py::class_<igris::buffer>(m, "igris_buffer")
	.def(py::init<const std::string&>())
	;

	py::class_<gateway> __gateway__(m, "gateway");
	py::class_<udpgate>(m, "udpgate", __gateway__)
	.def(py::init<>())
	.def(py::init<uint16_t>())
	.def("bind", &gateway::bind)
	;

	m.def("send", [](
	          const std::vector<uint8_t>& addr,
	          const std::string & data,
	          uint8_t type,
	          uint8_t qos,
	          uint16_t ackquant)
	{
		return crow::send(
		           crow::hostaddr(addr),
		           igris::buffer(data),
		           type,
		           qos,
		           ackquant
		       );
	});

	m.def("create_udpgate", &crow::create_udpgate,
	      py::return_value_policy::reference);
	m.def("onestep", &crow::onestep);
	m.def("spin", &crow::spin);
	m.def("start_spin", &crow::start_spin);
	m.def("stop_spin", &crow::stop_spin);

	//m.def("start_alive", (void(*)(const std::vector<uint8_t>&, const char*, uint16_t, uint16_t, uint8_t, uint16_t))&crow::start_alive, py::arg("addr"), py::arg("netname"), py::arg("resend_time"), py::arg("dietime"), py::arg("qos"), py::arg("ackquant"));
	//m.def("stop_alive", &crow::stop_alive);

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

	m.def("compile_address", &address);
	m.def("address", &address);

	m.def("diagnostic", &diagnostic_enable);
	m.def("live_diagnostic", &live_diagnostic_enable);

	//m.def("set_crowker", &set_crowker);
	m.def("envcrowker", &envcrowker);
	m.def("environment_crowker", &environment_crowker);

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

	m.def("diagnostic_enable", diagnostic_enable);
	m.def("finish", finish);

	py::class_<crow::node>(m, "node")
	.def("bind", py::overload_cast<>(&crow::node::bind))
	.def("bind", py::overload_cast<int>(&crow::node::bind))
	.def("send", [](
	         crow::node & self,
	         uint16_t rid,
	         const std::vector<uint8_t>& addr,
	         const std::string & data,
	         uint8_t qos,
	         uint16_t ackquant)
	{
		return self.send(
		           rid,
		           crow::hostaddr(addr),
		           igris::buffer(data),
		           qos,
		           ackquant
		       );
	})
	;

	py::class_<crow::pynode_delegate, crow::node>(m, "PyNode")
	.def(py::init <
	     std::function<void(crow::node_packet_ptr)>,
	     std::function<void(crow::node_packet_ptr)>
	     > ())
	;

	/*py::class_<crow::msgbox, crow::node>(m, "msgbox")
		.def(py::init<>())
		.def("send", py::overload_cast<const std::vector<uint8_t>&, uint16_t, const std::string&, uint8_t, uint16_t>(&crow::msgbox::send))
		.def("send", py::overload_cast<void*, uint8_t, uint16_t, void*, size_t, uint8_t, uint16_t>(&crow::msgbox::send))
		.def("query", &crow::msgbox::query, ungil())
		.def("receive", &crow::msgbox::receive, ungil())
	; */

	m.def("fully_empty", &crow::fully_empty);

	m.def("join_spin", &crow::join_spin, py::call_guard<py::gil_scoped_release>());
	m.def("start_spin", &crow::start_spin);
	m.def("stop_spin", &crow::stop_spin);
}