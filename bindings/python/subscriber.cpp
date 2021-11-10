#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <crow/nodes/subscriber_node.h>
#include <crow/hostaddr.h>
#include <pybind11/embed.h>

#include <functional>
#include <string>

namespace py = pybind11;

/*
class pybind_subscriber : public crow::subscriber
{
	std::function<void(crow::pubsub_packet_ptr)> delegate;
	std::string theme;
	crow::hostaddr addr;
	    
public:
	pybind_subscriber(std::function<void(crow::pubsub_packet_ptr)>& foo)
		: delegate(foo)
	{}

	void newpack_handler(crow::pubsub_packet_ptr ptr) override
	{
		delegate(ptr);
	}

	void subscribe(
	    const std::string& theme,
	    const crow::hostaddr& addr,
	    uint8_t qos,
	    uint16_t ackquant,
	    uint8_t rqos,
	    uint16_t rackquant
	)
	{
		this->addr = addr;
		this->theme = theme;

		crow::subscriber::subscribe(
			this->addr,
			this->theme.c_str(),
			qos, 
			ackquant,
			rqos,
			rackquant
		);
	}
};

void register_subscriber_class(py::module & m)
{
	py::class_<pybind_subscriber>(m, "subscriber")
	.def(py::init<std::function<void(crow::pubsub_packet_ptr)>&>())
	.def("subscribe", &pybind_subscriber::subscribe,
	     py::arg("addr"),
	     py::arg("theme"),
	     py::arg("ack") = 0,
	     py::arg("ackquant") = 0,
	     py::arg("rack") = 0,
	     py::arg("rackquant") = 0)
	.def("resubscribe", &pybind_subscriber::resubscribe)
	;
}
*/