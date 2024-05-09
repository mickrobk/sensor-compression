#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace sensor_compress {

class PointwiseCorrection {
 public:
  struct ReferenceValue {
    float observed;
    float actual;
  };
  explicit PointwiseCorrection(std::vector<ReferenceValue> references)
      : references_(std::move(references)) {}

  float Correct(float observed) {
    if (references_.empty()) {
      return observed;
    }
    auto it = std::lower_bound(references_.begin(), references_.end(), observed,
                               [](const ReferenceValue& a, float b) { return a.observed < b; });

    if (it == references_.begin()) {
      if (it->observed == observed) {
        return it->actual;
      }
      return observed;
    } else if (it == references_.end()) {
      return observed;
    }

    float lower = it[-1].actual;
    float upper = it->actual;
    float lowerBound = it[-1].observed;
    float upperBound = it->observed;

    float ratio = (observed - lowerBound) / (upperBound - lowerBound);
    return std::lerp(lower, upper, ratio);
  }

 private:
  std::vector<ReferenceValue> references_;
};

class LinearCorrection {
 public:
  LinearCorrection(uint16_t in_min, uint16_t in_max, float out_min, float out_max)
      : in_min_(in_min), in_max_(in_max), out_min_(out_min), out_max_(out_max) {}

  float Correct(uint16_t observed) {
    float a = in_min_;
    float b = in_max_;
    float c = out_min_;
    float d = out_max_;
    return c + (observed - a) / (b - a) * (d - c);
  }

 private:
  uint16_t in_min_;
  uint16_t in_max_;
  float out_min_;
  float out_max_;
};

class Sensor {
 public:
 private:
  uint64_t _uuid_;
  int version_;
  std::string name_;
  PointwiseCorrection correction_;
  uint16_t min_, max_;
  float scale_;
  uint16_t resolution_;
};

}  // namespace sensor_compress