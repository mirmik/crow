#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <crow/hostaddr.h>
#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/subscriber_node.h>
#include <crow/tower_cls.h>
#include <pybind11/embed.h>

#include <functional>
#include <string>

namespace py = pybind11;

#pragma GCC visibility push(hidden)
class pybind_subscriber : public crow::abstract_subscriber_node
{
    std::function<void(py::bytes)> delegate;
    std::string theme;
    crow::hostaddr addr;

public:
    pybind_subscriber(std::function<void(py::bytes)> &foo) : delegate(foo) {}

    void incoming_packet(crow::packet *pack) override
    {
        auto &s = pack->subheader<crow::pubsub_subheader>();

        switch (s.type)
        {
            case crow::PubSubTypes::Consume:
            {
                auto &sh = pack->subheader<crow::consume_subheader>();
                py::bytes data(sh.message().data(), sh.message().size());
                delegate(data);
            };
            break;

            default:
                break;
        }

        _tower->release(pack);
    }

    void subscribe(const crow::hostaddr &addr, const std::string &theme)
    {
        this->addr = addr;
        this->theme = theme;

        abstract_subscriber_node::init_subscribe(
            this->addr, this->theme.c_str(), 2, 50, 2, 50);
        abstract_subscriber_node::subscribe();
    }

    pybind_subscriber &bind_to_tower(crow::Tower &tower, int addr)
    {
        crow::node::bind(tower, addr);
        return *this;
    }

    pybind_subscriber &bind_to_tower_dynamic(crow::Tower &tower)
    {
        crow::node::bind(tower);
        return *this;
    }
};
#pragma GCC visibility pop

void register_subscriber_class(py::module &m)
{
    py::class_<pybind_subscriber, crow::node>(m, "subscriber")
        .def(py::init<std::function<void(py::bytes)> &>())
        .def("install_keepalive", &pybind_subscriber::install_keepalive)
        .def("subscribe", &pybind_subscriber::subscribe, py::arg("addr"),
             py::arg("theme"))
        .def("bind_to_tower", &pybind_subscriber::bind_to_tower,
             py::arg("tower"), py::arg("addr"),
             py::return_value_policy::reference)
        .def("bind_to_tower", &pybind_subscriber::bind_to_tower_dynamic,
             py::arg("tower"),
             py::return_value_policy::reference);
}
