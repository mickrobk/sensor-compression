#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#include <uuid.h>

#include <base64.hpp>

#include "compress.h"
#include "correction.h"
#include "data_frame.h"
#include "sensor.h"
#include "time_util.h"

namespace sensor_compress {
namespace json_internal {
inline std::string u64_to_b64(uint64_t u) {
  auto a = u64_to_bytes(u);
  return base64::encode_into<std::string>(a.begin(), a.end());
}

inline tl::expected<uint64_t, std::string> b64_to_u64(const std::string& s) {
  auto a = base64::decode_into<std::vector<uint8_t>>(s);
  return u64_from_bytes(a);
}
}  // namespace json_internal
}  // namespace sensor_compress

namespace nlohmann {
template <>
struct adl_serializer<sensor_compress::utc_time_point_t> {
  static void to_json(Json& j, const sensor_compress::utc_time_point_t& p) {
    j = sensor_compress::ToMs(p);
  }

  static void from_json(const Json& j, sensor_compress::utc_time_point_t& p) {
    p = sensor_compress::UTCFromMs(j.template get<uint64_t>());
  }
};

template <>
struct adl_serializer<sensor_compress::steady_time_point_t> {
  static void to_json(Json& j, const sensor_compress::steady_time_point_t& p) {
    j = sensor_compress::ToMs(p);
  }

  static void from_json(const Json& j, sensor_compress::steady_time_point_t& p) {
    p = sensor_compress::SteadyFromMs(j.template get<uint64_t>());
  }
};

template <>
struct adl_serializer<uuids::uuid> {
  static void to_json(Json& j, const uuids::uuid& u) { j = uuids::to_string(u); }

  static void from_json(const Json& j, uuids::uuid& u) {
    u = {};

    if (auto parsed = uuids::uuid::from_string(j.template get<std::string>())) {
      u = *parsed;
    }
  }
};

template <>
struct adl_serializer<sensor_compress::CompressedDataFrame> {
  static void to_json(Json& j, const sensor_compress::CompressedDataFrame& p) {
    auto sc_bytes = sensor_compress::to_bytes(p.side_channel);
    j["values"] = base64::encode_into<std::string>(p.values.begin(), p.values.end());
    j["times"] = base64::encode_into<std::string>(p.times.begin(), p.times.end());
    j["side_channel"] = base64::encode_into<std::string>(sc_bytes.begin(), sc_bytes.end());
  }

  static void from_json(const Json& j, sensor_compress::CompressedDataFrame& p) {
    auto tvals = j.find("times");
    auto scvals = j.find("side_channel");
    auto vvals = j.find("values");
    if (tvals == j.end() || scvals == j.end() || vvals == j.end()) return;
    auto sc_or = sensor_compress::from_bytes(
        base64::decode_into<std::vector<uint8_t>>(scvals->template get<std::string>()));
    if (!sc_or) return;
    p.values = base64::decode_into<std::vector<uint8_t>>(vvals->template get<std::string>());
    p.times = base64::decode_into<std::vector<uint8_t>>(tvals->template get<std::string>());
    p.side_channel = *sc_or;
  }
};

}  // namespace nlohmann

namespace sensor_compress {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DataHeader,          //
                                   name,                //
                                   session_id,          //
                                   version,             //
                                   min,                 //
                                   max,                 //
                                   resolution_bits,     //
                                   frame_size,          //
                                   value_compressions,  //
                                   time_compressions,   //
                                   start_time_utc,      //
                                   start_time_steady    //
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompressedSensorReadings,  //
                                   header,                    //
                                   frames                     //
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DataFrameValue,  //
                                   t,               //
                                   value            //
);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SensorReadings,  //
                                   header,          //
                                   values           //
);

}  // namespace sensor_compress