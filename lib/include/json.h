#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#include "correction.h"
#include "data_frame.h"
#include "sensor.h"
#include "time_util.h"
#include "uuid.h"

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
}  // namespace nlohmann

namespace sensor_compress {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Uuid,  //
                                   bytes  //
);

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