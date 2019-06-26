#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <crow/gates/udpgate.h>
#include <crow/holders.h>
#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/hexer.h>

//#include <igris/print.h>

using namespace crow;
namespace py = pybind11;

py::function incoming_handler_bind;
void incoming_handler_bind_invoke(crow::packet *pack)
{
	crow::packref control(pack);
	incoming_handler_bind(control);
}

py::function subscribe_handler_bind;
void subscribe_handler_bind_invoke(crow::packet *pack)
{
	crow::pubsub_data_packref control(pack);
	subscribe_handler_bind(control);
}

PYBIND11_MODULE(libcrow, m)
{
	py::class_<packref>(m, "packref")
		.def("rawdata",
			 [](packref &self) -> py::bytes {
				 auto buf = self.rawdata();
				 return {buf.data(), buf.size()};
			 })
		.def("addr", [](packref &self) -> py::bytes {
			auto buf = self.addr();
			return {buf.data(), buf.size()};
		});

	py::class_<pubsub_packref, packref>(m, "pubsub_packref")
		.def("theme", [](pubsub_packref &self) -> py::str {
			auto buf = self.theme();
			return {buf.data(), buf.size()};
		});

	py::class_<pubsub_data_packref, pubsub_packref>(m, "pubsub_data_packref")
		.def("data", [](pubsub_data_packref &self) -> py::bytes {
			auto buf = self.data();
			return {buf.data(), buf.size()};
		});

	py::class_<gateway> __gateway__(m, "gateway");
	py::class_<udpgate>(m, "udpgate", __gateway__);

	m.def("create_udpgate", &crow::create_udpgate,
		  py::return_value_policy::reference);
	m.def("onestep", &crow::onestep);
	m.def("spin", &crow::spin);

	m.def("set_incoming_handler", [](py::function f) {
		incoming_handler_bind = f;
		crow::user_incoming_handler = incoming_handler_bind_invoke;
	});

	m.def("set_subscribe_handler", [](py::function f) {
		subscribe_handler_bind = f;
		crow::pubsub_protocol.enable();
		crow::pubsub_protocol.incoming_handler = subscribe_handler_bind_invoke;
	});

	m.def("compile_address", &compile_address);

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
	py::capsule cleanup(&unused, [](void *) {
		user_incoming_handler = nullptr;
		incoming_handler_bind.release();

		crow::pubsub_protocol.incoming_handler = nullptr;
		subscribe_handler_bind.release();
	});
	m.add_object("_cleanup", cleanup);

	m.def("diagnostic_enable", diagnostic_enable);
}