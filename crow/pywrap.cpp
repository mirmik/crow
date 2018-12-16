#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <crow/tower.h>

namespace py = pybind11;

PYBIND11_MODULE(libcrow, m)
{
	m.def("spin", &crow::spin);
}