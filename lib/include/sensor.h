#pragma once

#include <json.h>

#include <cstdint>
#include <string>
#include <tl/expected.hpp>

#include "correction.h"
#include "data_frame.h"
#include "data_header.h"
#include "data_stream.h"

namespace sensor_compress {

template <typename TString>
class Sensor {
 protected:
  Sensor(DataHeader header, CombinedCorrection correction = CombinedCorrection())
      : header_(header), correction_(correction) {}

 public:
  virtual ~Sensor() = default;
  tl::expected<void, std::string> Update() {
    if (auto value = GetValue()) {
      last_ = *value;
      auto maybe_compressed = stream_.Record(header_, *value);
      if (auto compressed = maybe_compressed) {
        if (!compressed) return tl::unexpected<std::string>(compressed.error());

        // Todo: get the result from compressed and convert it to json
        // Then: copy that result json the TString type, then put it into the back
        // of compressed_values_
      }
    }
  }
  std::optional<DataFrameValue> GetLast() const { return last_; }

 protected:
  virtual std::optional<DataFrameValue> GetValue() = 0;

  DataFrameValue last_;
  DataHeader header_;
  CombinedCorrection correction_;
  DataStream stream_;
  std::vector<TString> compressed_values_;
};

}  // namespace sensor_compress