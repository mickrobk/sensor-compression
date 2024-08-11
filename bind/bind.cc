#include <data_frame.h>
#include <data_header.h>
#include <json.h>
#include <pybind11/pybind11.h>
#include <sensor.h>

#include <nlohmann/json.hpp>

namespace py = pybind11;
using namespace sensor_compress;

std::string decompressSensorReadings(std::string compressed) {
  auto json_compressed = nlohmann::json::parse(compressed);
  auto compressed_readings = json_compressed.template get<CompressedSensorReadings>();
  auto decompressed_readings = compressed_readings.Decompress();
  if (!decompressed_readings) {
    throw std::runtime_error("Decompress failed: " + decompressed_readings.error());
  }
  return Json(*decompressed_readings).dump();
}

int add(int i, int j) { return i + j; }

PYBIND11_MODULE(sensor_compress, m) {
  m.doc() = "sensor_compress python bindings";  // optional module docstring

  m.def("add", &add, "A function that adds two numbers");
  m.def("decompressSensorReadings", &decompressSensorReadings,
        "JSON CompressedSensorReadings --> JSON SensorReadings");
}