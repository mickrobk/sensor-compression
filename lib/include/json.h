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

  // To store as a string...
  // static void to_json(Json& j, const sensor_compress::utc_time_point_t& p) {
  //   j = sensor_compress::json_internal::u64_to_b64(sensor_compress::ToMs(p));
  // }

  // static void from_json(const Json& j, sensor_compress::utc_time_point_t& p) {
  //   auto u64_or = sensor_compress::json_internal::b64_to_u64(j.template get<std::string>());
  //   if (u64_or) {
  //     p = sensor_compress::UTCFromMs(*u64_or);
  //   }
  // }
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CompressedDataFrame,  //
                                   side_channel,         //
                                   values,               //
                                   times                 //
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