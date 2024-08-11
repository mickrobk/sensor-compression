
template <typename TString>
tl::expected<void, std::string> Sensor<TString>::Update(steady_time_point_t t) {
  if (current_frame_.size() >= header_.frame_size) {
    auto compressed_current = current_frame_.Compress(header_);
    current_frame_.Clear();
    if (!compressed_current) return tl::unexpected{compressed_current.error()};
    compressed_values_.push_back(std::move(compressed_current.value()));
  }

  auto value = GetValue(t);
  if (!value) return {};

  current_frame_.Record(*value);
  last_ = *value;
  return {};
}

template <typename TString>
CompressedSensorReadings Sensor<TString>::TakeReadings() {
  CompressedSensorReadings readings;
  readings.header = header_;
  readings.frames = std::move(compressed_values_);
  compressed_values_.clear();
  return readings;
}
