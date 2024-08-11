#include <data_frame.h>
#include <data_header.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) {
  sensor_compress::DataHeader header{0, 10000};
  return i + j;
}

PYBIND11_MODULE(sensor_compress, m) {
  m.doc() = "pybind11 example plugin";  // optional module docstring

  m.def("add", &add, "A function that adds two numbers");
}