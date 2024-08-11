#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

struct Uuid {
  std::array<uint8_t, 16> bytes;

  // Generates a random UUID
  static Uuid Generate() {
    Uuid uuid;
    std::random_device rd;   // Seed for the random number generator
    std::mt19937 gen(rd());  // Mersenne Twister engine
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    for (auto& byte : uuid.bytes) {
      byte = dis(gen);
    }

    // Set the version (4) and variant (2) bits
    uuid.bytes[6] = (uuid.bytes[6] & 0x0F) | 0x40;  // Version 4
    uuid.bytes[8] = (uuid.bytes[8] & 0x3F) | 0x80;  // Variant 1

    return uuid;
  }

  // Converts the UUID to a string
  std::string to_string() const {
    std::stringstream ss;
    for (size_t i = 0; i < bytes.size(); ++i) {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
      if (i == 3 || i == 5 || i == 7 || i == 9) {
        ss << "-";
      }
    }
    return ss.str();
  }
};