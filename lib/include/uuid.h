#pragma once

#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace sensor_compress {
struct Uuid {
  std::array<uint8_t, 16> bytes;

  // Generates a random UUID
  static Uuid Generate() {
    thread_local static std::random_device rd;   // Seed for the random number generator
    thread_local static std::mt19937 gen(rd());  // Mersenne Twister engine
    thread_local static std::uniform_int_distribution<uint8_t> dis(0, 255);

    Uuid uuid;
    for (auto& byte : uuid.bytes) {
      byte = dis(gen);
    }

    // Set the version (4) and variant (2) bits
    uuid.bytes[6] = (uuid.bytes[6] & 0x0F) | 0x40;  // Version 4
    uuid.bytes[8] = (uuid.bytes[8] & 0x3F) | 0x80;  // Variant 1

    return uuid;
  }
};
}  // namespace sensor_compress