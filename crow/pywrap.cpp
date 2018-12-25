#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

namespace py = pybind11;

PYBIND11_MODULE(libcrow, m)
{
	m.def("create_udpgate", &crow::create_udpgate);
	m.def("spin", &crow::spin);
}