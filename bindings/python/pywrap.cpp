#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <crow/brocker/crowker.h>
#include <crow/nodes/pubsub_defs.h>

#include "pynode.h"
#include <crow/address.h>
#include <crow/gates/loopgate.h>
#include <crow/gates/tcpgate.h>
#include <crow/gates/udpgate.h>
#include <crow/iter.h>
#include <crow/nodes/publisher_node.h>
#include <crow/nodes/requestor_node.h>
#include <crow/nodes/subscriber_node.h>
#include <crow/packet_ptr.h>
#include <crow/proto/msgbox.h>
#include <crow/proto/node.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/tower_thread_executor.h>

using namespace crow;
namespace py = pybind11;
using ungil = py::call_guard<py::gil_scoped_release>;

void register_subscriber_class(py::module &m);
void register_publisher_class(py::module &m);
void register_requestor_class(py::module &m);

// Helper class for udpgate with Tower binding support
class py_udpgate : public crow::udpgate
{
public:
    using crow::udpgate::udpgate;

    int bind_to_tower(crow::Tower &tower, int gate_no = CROW_UDPGATE_NO)
    {
        return crow::gateway::bind(tower, gate_no);
    }
};

// Helper class for loopgate with Tower binding support
class py_loopgate : public crow::loopgate
{
public:
    using crow::loopgate::loopgate;

    int bind_to_tower(crow::Tower &tower, int gate_no)
    {
        return crow::gateway::bind(tower, gate_no);
    }
};

// Helper class for tcpgate with Tower binding support
class py_tcpgate : public crow::tcpgate
{
public:
    using crow::tcpgate::tcpgate;

    int bind_to_tower(crow::Tower &tower, int gate_no = CROW_TCPGATE_NO)
    {
        return crow::gateway::bind(tower, gate_no);
    }
};

PYBIND11_MODULE(libcrow, m)
{
    // Tower class - for multiple independent towers
    py::class_<crow::Tower>(m, "Tower")
        .def(py::init<>())
        .def("send",
             [](crow::Tower &self, const crow::hostaddr_view &addr,
                const std::string &data, uint8_t type, uint8_t qos,
                uint16_t ackquant, bool async) {
                 return self.send(addr, data, type, qos, ackquant, async);
             },
             py::arg("addr"), py::arg("data"), py::arg("type") = 0,
             py::arg("qos") = 2, py::arg("ackquant") = 50,
             py::arg("async") = false)
        .def("onestep", &crow::Tower::onestep)
        .def("has_untravelled", &crow::Tower::has_untravelled)
        .def("get_total_travelled", &crow::Tower::get_total_travelled)
        .def("set_retransling_allowed", &crow::Tower::set_retransling_allowed)
        .def("get_retransling_allowed", &crow::Tower::get_retransling_allowed)
        .def("set_debug_data_size", &crow::Tower::set_debug_data_size)
        .def("get_debug_data_size", &crow::Tower::get_debug_data_size)
        .def("enable_diagnostic", &crow::Tower::enable_diagnostic)
        .def("diagnostic_enabled", &crow::Tower::diagnostic_enabled)
        .def("release",
             [](crow::Tower &self, crow::packet *pack) { self.release(pack); });

    auto pack = py::class_<packet_ptr>(m, "packet_ptr")
                    .def("rawdata",
                         [](packet_ptr &self) -> py::bytes {
                             nos::buffer buf = self->data();
                             return {buf.data(), buf.size()};
                         })
                    .def("message",
                         [](packet_ptr &self) -> py::bytes {
                             // Extract message from consume subheader
                             auto &sh = self->subheader<crow::consume_subheader>();
                             nos::buffer buf = sh.message();
                             return {buf.data(), buf.size()};
                         })
                    .def("addr", [](packet_ptr &self) -> crow::hostaddr {
                        return self->addr();
                    });

    py::class_<node_packet_ptr>(m, "node_packet_ptr", pack)
        .def("message", [](node_packet_ptr &self) -> py::bytes {
            auto buf = self.message();
            return {buf.data(), buf.size()};
        });

    py::class_<crow::hostaddr>(m, "hostaddr")
        .def(py::init<const crow::hostaddr_view &>())
        .def(py::init<const std::vector<uint8_t> &>())
        .def(py::init<const std::string &>())
        .def("view",
             (crow::hostaddr_view(crow::hostaddr::*)()) & crow::hostaddr::view);
    py::implicitly_convertible<std::string, crow::hostaddr>();

    py::class_<crow::hostaddr_view>(m, "hostaddr_view")
        .def(py::init<const crow::hostaddr &>())
        .def(py::init<const std::vector<uint8_t> &>());
    py::implicitly_convertible<crow::hostaddr, crow::hostaddr_view>();

    auto __gateway__ = py::class_<gateway>(m, "gateway")
                           .def("finish", &gateway::finish);

    py::class_<py_udpgate>(m, "udpgate", __gateway__)
        .def(py::init<>())
        .def(py::init<uint16_t>())
        .def("open", &py_udpgate::open)
        .def("close", &py_udpgate::close)
        .def("debug", &py_udpgate::debug)
        .def("read_handler", &py_udpgate::read_handler, py::arg("fd") = 0)
        .def("bind_to_tower", &py_udpgate::bind_to_tower,
             py::arg("tower"), py::arg("gate_no") = CROW_UDPGATE_NO);

    py::class_<py_loopgate>(m, "loopgate", __gateway__)
        .def(py::init<>())
        .def("bind_to_tower", &py_loopgate::bind_to_tower,
             py::arg("tower"), py::arg("gate_no"));

    py::class_<py_tcpgate>(m, "tcpgate", __gateway__)
        .def(py::init<>())
        .def("open", &py_tcpgate::open, py::arg("port"))
        .def("close", &py_tcpgate::close)
        .def("debug", &py_tcpgate::debug, py::arg("en"))
        .def("bind_to_tower", &py_tcpgate::bind_to_tower,
             py::arg("tower"), py::arg("gate_no") = CROW_TCPGATE_NO);

    m.def("crowker_address", &crow::crowker_address);

    m.def("address", [](std::string str) { return crow::address(str); });

    py::class_<crow::node>(m, "node")
        .def("bind_to_tower",
             [](crow::node &self, crow::Tower &tower, int addr) -> crow::node & {
                 return self.bind(tower, addr);
             },
             py::arg("tower"), py::arg("addr"),
             py::return_value_policy::reference)
        .def("bind_to_tower",
             [](crow::node &self, crow::Tower &tower) -> crow::node & {
                 return self.bind(tower);
             },
             py::arg("tower"),
             py::return_value_policy::reference)
        .def_readwrite("id", &node::id)
        .def(
            "send",
            [](crow::node &node, int rid, const crow::hostaddr_view &addr,
               const std::string &data, uint8_t qos, uint16_t ackquant,
               bool fastsend) {
                return node.send(rid, addr, data, qos, ackquant);
            },
            ungil(), py::arg("rid"), py::arg("addr"), py::arg("data"),
            py::arg("qos") = 2, py::arg("ackquant") = 50,
            py::arg("fastsend") = false);

    py::class_<crow::pynode_delegate, crow::node>(m, "PyNode")
        .def(py::init<std::function<void(crow::node_packet_ptr)>,
                      std::function<void(crow::node_packet_ptr)>>());

    m.def("join_spin", &crow::join_spin,
          py::call_guard<py::gil_scoped_release>());
    m.def("stop_spin", &crow::stop_spin, py::arg("wait") = true);
    m.def("spin", [](crow::Tower &tower) { crow::spin(tower); },
          py::arg("tower"));

    // TowerThreadExecutor class
    py::class_<crow::TowerThreadExecutor>(m, "TowerThreadExecutor")
        .def(py::init<crow::Tower &>(), py::arg("tower"))
        .def("start", &crow::TowerThreadExecutor::start)
        .def("stop", &crow::TowerThreadExecutor::stop, py::arg("wait") = true,
             py::call_guard<py::gil_scoped_release>())
        .def("join", &crow::TowerThreadExecutor::join,
             py::call_guard<py::gil_scoped_release>())
        .def("running", &crow::TowerThreadExecutor::running);

    register_subscriber_class(m);
    register_publisher_class(m);
    register_requestor_class(m);
}
