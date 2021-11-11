#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <crow/nodes/subscriber_node.h>
#include <crow/nodes/pubsub_defs.h>
#include <crow/hostaddr.h>
#include <pybind11/embed.h>

#include <functional>
#include <string>

namespace py = pybind11;

#pragma GCC visibility push(hidden)
class pybind_publisher : public crow::publisher_node
{
	std::string theme;
	crow::hostaddr addr;

public:
	pybind_publisher(crow::hostaddr addr, std::string theme)
		: crow::publisher_node()
	{
		this->addr = addr;
		this-> theme = theme;
		set_theme(this->theme);
		set_address(this->addr);
	}

	void publish(
	    const py::bytes& data
	)
	{
		std::string info = data;
		publisher_node::publish({ 
			info.data(), 
			info.size() });
	}
};
#pragma GCC visibility pop

void register_publisher_class(py::module & m)
{
	py::class_<pybind_publisher>(m, "publisher")
	.def(py::init<crow::hostaddr, std::string>(), py::arg("addr"), py::arg("theme"))
	.def("publish", &pybind_publisher::publish, py::arg("data"))
	;
}
