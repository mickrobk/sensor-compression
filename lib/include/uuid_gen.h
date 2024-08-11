#pragma once

#include <uuid.h>

#include <algorithm>

namespace sensor_compress {

class UuidGen {
 public:
  static uuids::uuid Generate() {
    static thread_local std::unique_ptr<UuidGen> gen;
    if (!gen) {
      gen = std::make_unique<UuidGen>();
    }
    return (*gen)();
  }

  UuidGen() {
    std::generate(std::begin(seed_data_), std::end(seed_data_), std::ref(rd_));
    seq_ = std::make_unique<std::seed_seq>(std::begin(seed_data_), std::end(seed_data_));
    generator_ = std::mt19937(*seq_);
    gen_ = std::make_unique<uuids::uuid_random_generator>(generator_);
  }

  uuids::uuid operator()() { return (*gen_)(); }

 private:
  std::random_device rd_;
  std::array<int, std::mt19937::state_size> seed_data_;
  std::unique_ptr<std::seed_seq> seq_;
  std::mt19937 generator_;
  std::unique_ptr<uuids::uuid_random_generator> gen_;
};

}  // namespace sensor_compress