#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "lerp.h"

namespace sensor_compress {

template <typename T>
class TLinearTransform {
 public:
  TLinearTransform(T in_min, T in_max, T out_min, T out_max)
      : in_min_(in_min), in_max_(in_max), out_min_(out_min), out_max_(out_max) {}

  T operator()(T observed) const {
    T a = in_min_;
    T b = in_max_;
    T c = out_min_;
    T d = out_max_;
    return c + (observed - a) / (b - a) * (d - c);
  }

 private:
  T in_min_;
  T in_max_;
  T out_min_;
  T out_max_;
};

class Corrector {
 public:
  virtual ~Corrector() = default;
  virtual float Correct(float observed) const = 0;
};

class PointwiseCorrection : public Corrector {
 public:
  virtual ~PointwiseCorrection() = default;
  struct ReferenceValue {
    float observed;
    float actual;
  };
  explicit PointwiseCorrection(std::vector<ReferenceValue> references)
      : references_(std::move(references)) {}

  float Correct(float observed) const override {
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
    return lerp(lower, upper, ratio);
  }

 private:
  std::vector<ReferenceValue> references_;
};

class LinearCorrection : public Corrector {
 public:
  virtual ~LinearCorrection() = default;
  LinearCorrection(float in_min, float in_max, float out_min, float out_max)
      : func_(in_min, in_max, out_min, out_max) {}
  float Correct(float observed) const override { return func_(observed); }

 private:
  TLinearTransform<float> func_;
};

class CombinedCorrection : public Corrector {
 public:
  float Correct(float observed) const override {
    for (auto& c : correctors_) {
      observed = c->Correct(observed);
    }
    return observed;
  }

 private:
  std::vector<std::unique_ptr<Corrector>> correctors_;
};

}  // namespace sensor_compress