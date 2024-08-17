
/*
    Adapted from https://github.com/lemire/FastPFor (Apache License Version 2.0)

    Implements Simple8b integer compression/decompression as described in original paper:
        Vo Ngoc Anh, Alistair Moffat: Index compression using 64-bit words
        Softw., Pract. Exper. 40(2): 131-147 (2010)

    Notable changes:
        - C++ templates used to make methods generic over integer bit-width
        - support longer arrays via 64 bit length arguments

    --------------------------------------------------------------------------------
    --------------------------------------------------------------------------------

    Adapted from https://github.com/naturalplasmoid/simple8b-timeseries-compression
    (Apache License Version 2.0) by Robert Mickle

    Notable changes:
      - Integer safe operations for delta encode/decode
*/

inline void to_bytes(uint64_t x, uint8_t *b) {
  b[0] = x >> 8 * 0;
  b[1] = x >> 8 * 1;
  b[2] = x >> 8 * 2;
  b[3] = x >> 8 * 3;
  b[4] = x >> 8 * 4;
  b[5] = x >> 8 * 5;
  b[6] = x >> 8 * 6;
  b[7] = x >> 8 * 7;
}
inline std::vector<uint8_t> to_bytes(std::vector<uint64_t> v) {
  std::vector<uint8_t> result(v.size() * 8);
  for (size_t i = 0; i < v.size(); i++) {
    to_bytes(v[i], result.data() + i * 8);
  }
  return result;
}
tl::expected<std::vector<uint64_t>, std::string> from_bytes(const std::vector<uint8_t> &b) {
  if (b.size() % 8 != 0) {
    return tl::make_unexpected("Invalid length");
  }
  std::vector<uint64_t> result(b.size() / 8);
  for (size_t i = 0; i < result.size(); i++) {
    result[i] = (static_cast<uint64_t>(b[i * 8 + 0]) << 8 * 0) | (static_cast<uint64_t>(b[i * 8 + 1]) << 8 * 1) |
                (static_cast<uint64_t>(b[i * 8 + 2]) << 8 * 2) | (static_cast<uint64_t>(b[i * 8 + 3]) << 8 * 3) |
                (static_cast<uint64_t>(b[i * 8 + 4]) << 8 * 4) | (static_cast<uint64_t>(b[i * 8 + 5]) << 8 * 5) |
                (static_cast<uint64_t>(b[i * 8 + 6]) << 8 * 6) | (static_cast<uint64_t>(b[i * 8 + 7]) << 8 * 7);
  }
  return result;
}

const uint8_t SIMPLE8B_SELECTOR_BITS =
    4;  // number of bits used by Simple8b algorithm to indicate packing scheme

template <typename T>
static void WriteBits(uint64_t *out, const T value, const uint32_t numBits) {
  *out = (*out << numBits) | value;
}

template <size_t numIntegers, uint32_t numBitsPerInt, typename T>
static bool TryPackFast(const T *n) {
  if (numBitsPerInt >= 32) return true;
  for (size_t i = 0; i < numIntegers; i++) {
    if (n[i] >= (1ULL << numBitsPerInt)) return false;
  }
  return true;
}

template <size_t numIntegers, uint32_t numBitsPerInt, typename T>
static bool TryPackCareful(const T *n, size_t maxIntegers) {
  if (numBitsPerInt >= 32) return true;
  const size_t minv = (maxIntegers < numIntegers) ? maxIntegers : numIntegers;
  for (size_t i = 0; i < minv; i++) {
    if (n[i] >= (1ULL << numBitsPerInt)) return false;
  }
  return true;
}

template <uint32_t numIntegers, uint32_t numBitsPerInt, typename T>
static void UnpackFast(T *&out, const uint64_t *&in) {
  const uint64_t mask = (1ULL << numBitsPerInt) - 1;
  if (numBitsPerInt < 32) {
    for (uint32_t k = 0; k < numIntegers; ++k) {
      *(out++) = static_cast<T>(in[0] >>
                                (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                 mask;
    }
  } else {
    for (uint32_t k = 0; k < numIntegers; ++k) {
      *(out++) = static_cast<T>(in[0] >>
                                (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                 mask;
    }
  }
  ++in;
}

template <uint32_t numBitsPerInt, typename T>
static void UnpackCareful(uint32_t numIntegers, T *&out, const uint64_t *&in) {
  const uint64_t mask = (1ULL << numBitsPerInt) - 1;
  if (numBitsPerInt < 32) {
    for (uint32_t k = 0; k < numIntegers; ++k) {
      *(out++) = static_cast<T>(in[0] >>
                                (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                 mask;
    }
  } else {
    for (uint32_t k = 0; k < numIntegers; ++k) {
      *(out++) = static_cast<T>(in[0] >>
                                (64 - SIMPLE8B_SELECTOR_BITS - numBitsPerInt - k * numBitsPerInt)) &
                 mask;
    }
  }
  ++in;
}

inline uint32_t GetSelectorNum(const uint64_t *const in) {
  return static_cast<uint32_t>((*in) >> (64 - SIMPLE8B_SELECTOR_BITS));
}

template <typename T>
size_t Simple8bEncode(T *input, size_t inputLength, uint64_t *out) {
  uint32_t numberOfValuesCoded = 0;
  const uint64_t *const initout = out;
  size_t valuesRemaining(inputLength);

  while (valuesRemaining >= 240) {
    if (TryPackFast<120, 0>(input)) {
      if (TryPackFast<120, 0>(input + 120)) {
        numberOfValuesCoded = 240;
        out[0] = 0;
        input += numberOfValuesCoded;
      } else {
        numberOfValuesCoded = 120;
        out[0] = 1ULL << (64 - SIMPLE8B_SELECTOR_BITS);
        input += numberOfValuesCoded;
      }
    } else if (TryPackFast<60, 1>(input)) {
      out[0] = 2;
      numberOfValuesCoded = 60;
      for (uint32_t i = 0; i < 60; i++) {
        WriteBits(out, *input++, 1);
      }
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 1 * 60;
    } else if (TryPackFast<30, 2>(input)) {
      out[0] = 3;
      numberOfValuesCoded = 30;
      for (uint32_t i = 0; i < 30; i++) WriteBits(out, *input++, 2);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 2 * 30;
    } else if (TryPackFast<20, 3>(input)) {
      out[0] = 4;
      numberOfValuesCoded = 20;
      for (uint32_t i = 0; i < 20; i++) WriteBits(out, *input++, 3);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 3 * 20;
    } else if (TryPackFast<15, 4>(input)) {
      out[0] = 5;
      numberOfValuesCoded = 15;
      for (uint32_t i = 0; i < 15; i++) WriteBits(out, *input++, 4);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 4 * 15;
    } else if (TryPackFast<12, 5>(input)) {
      out[0] = 6;
      numberOfValuesCoded = 12;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 5);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 5 * 12;
    } else if (TryPackFast<10, 6>(input)) {
      out[0] = 7;
      numberOfValuesCoded = 10;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 6);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 6 * 10;
    } else if (TryPackFast<8, 7>(input)) {
      out[0] = 8;
      numberOfValuesCoded = 8;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 7);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 7 * 8;
    } else if (TryPackFast<7, 8>(input)) {
      out[0] = 9;
      numberOfValuesCoded = 7;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 8);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 8 * 7;
    } else if (TryPackFast<6, 10>(input)) {
      out[0] = 10;
      numberOfValuesCoded = 6;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 10);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 10 * 6;
    } else if (TryPackFast<5, 12>(input)) {
      out[0] = 11;
      numberOfValuesCoded = 5;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 12);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 12 * numberOfValuesCoded;
    } else if (TryPackFast<4, 15>(input)) {
      out[0] = 12;
      numberOfValuesCoded = 4;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 15);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 15 * 4;
    } else if (TryPackFast<3, 20>(input)) {
      out[0] = 13;
      numberOfValuesCoded = 3;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 20);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 20 * 3;
    } else if (TryPackFast<2, 30>(input)) {
      out[0] = 14;
      numberOfValuesCoded = 2;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 30);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 30 * 2;
    } else if (TryPackFast<1, 60>(input)) {
      out[0] = 15;
      numberOfValuesCoded = 1;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 60);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 60 * 1;
    } else {
      std::abort();
    }
    ++out;

    valuesRemaining -= numberOfValuesCoded;
  }
  while (valuesRemaining > 0) {
    if (TryPackCareful<240, 0>(input, valuesRemaining)) {
      numberOfValuesCoded = (valuesRemaining < 240) ? static_cast<uint32_t>(valuesRemaining) : 240;
      out[0] = 0;
      input += numberOfValuesCoded;
    } else if (TryPackCareful<120, 0>(input, valuesRemaining)) {
      numberOfValuesCoded = (valuesRemaining < 120) ? static_cast<uint32_t>(valuesRemaining) : 120;
      out[0] = 1ULL << (64 - SIMPLE8B_SELECTOR_BITS);
      input += numberOfValuesCoded;
    } else if (TryPackCareful<60, 1>(input, valuesRemaining)) {
      out[0] = 2;
      numberOfValuesCoded = (valuesRemaining < 60) ? static_cast<uint32_t>(valuesRemaining) : 60;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) {
        WriteBits(out, *input++, 1);
      }
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 1 * numberOfValuesCoded;
    } else if (TryPackCareful<30, 2>(input, valuesRemaining)) {
      out[0] = 3;
      numberOfValuesCoded = (valuesRemaining < 30) ? static_cast<uint32_t>(valuesRemaining) : 30;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 2);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 2 * numberOfValuesCoded;
    } else if (TryPackCareful<20, 3>(input, valuesRemaining)) {
      out[0] = 4;
      numberOfValuesCoded = (valuesRemaining < 20) ? static_cast<uint32_t>(valuesRemaining) : 20;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 3);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 3 * numberOfValuesCoded;
    } else if (TryPackCareful<15, 4>(input, valuesRemaining)) {
      out[0] = 5;
      numberOfValuesCoded = (valuesRemaining < 15) ? static_cast<uint32_t>(valuesRemaining) : 15;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 4);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 4 * numberOfValuesCoded;
    } else if (TryPackCareful<12, 5>(input, valuesRemaining)) {
      out[0] = 6;
      numberOfValuesCoded = (valuesRemaining < 12) ? static_cast<uint32_t>(valuesRemaining) : 12;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 5);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 5 * numberOfValuesCoded;
    } else if (TryPackCareful<10, 6>(input, valuesRemaining)) {
      out[0] = 7;
      numberOfValuesCoded = (valuesRemaining < 10) ? static_cast<uint32_t>(valuesRemaining) : 10;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 6);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 6 * numberOfValuesCoded;
    } else if (TryPackCareful<8, 7>(input, valuesRemaining)) {
      out[0] = 8;
      numberOfValuesCoded = (valuesRemaining < 8) ? static_cast<uint32_t>(valuesRemaining) : 8;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 7);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 7 * numberOfValuesCoded;
    } else if (TryPackCareful<7, 8>(input, valuesRemaining)) {
      out[0] = 9;
      numberOfValuesCoded = (valuesRemaining < 7) ? static_cast<uint32_t>(valuesRemaining) : 7;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 8);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 8 * numberOfValuesCoded;
    } else if (TryPackCareful<6, 10>(input, valuesRemaining)) {
      out[0] = 10;
      numberOfValuesCoded = (valuesRemaining < 6) ? static_cast<uint32_t>(valuesRemaining) : 6;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 10);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 10 * numberOfValuesCoded;
    } else if (TryPackCareful<5, 12>(input, valuesRemaining)) {
      out[0] = 11;
      numberOfValuesCoded = (valuesRemaining < 5) ? static_cast<uint32_t>(valuesRemaining) : 5;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 12);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 12 * numberOfValuesCoded;
    } else if (TryPackCareful<4, 15>(input, valuesRemaining)) {
      out[0] = 12;
      numberOfValuesCoded = (valuesRemaining < 4) ? static_cast<uint32_t>(valuesRemaining) : 4;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 15);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 15 * numberOfValuesCoded;
    } else if (TryPackCareful<3, 20>(input, valuesRemaining)) {
      out[0] = 13;
      numberOfValuesCoded = (valuesRemaining < 3) ? static_cast<uint32_t>(valuesRemaining) : 3;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 20);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 20 * numberOfValuesCoded;
    } else if (TryPackCareful<2, 30>(input, valuesRemaining)) {
      out[0] = 14;
      numberOfValuesCoded = (valuesRemaining < 2) ? static_cast<uint32_t>(valuesRemaining) : 2;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 30);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 30 * numberOfValuesCoded;
    } else if (TryPackCareful<1, 60>(input, valuesRemaining)) {
      out[0] = 15;
      numberOfValuesCoded = (valuesRemaining < 1) ? valuesRemaining : 1;
      for (uint32_t i = 0; i < numberOfValuesCoded; i++) WriteBits(out, *input++, 60);
      out[0] <<= 64 - SIMPLE8B_SELECTOR_BITS - 60 * numberOfValuesCoded;
    } else {
      std::abort();
    }

    ++out;

    valuesRemaining -= numberOfValuesCoded;
  }

  return (out)-initout;
}

template <typename T>
size_t Simple8bDecode(uint64_t *input, size_t uncompressedLength, T *out) {
  const uint64_t *in = input;
  const T *const end = out + uncompressedLength;
  const T *const initout = out;

  while (end > out + 240) {
    switch (GetSelectorNum(in)) {
      case 0:
        UnpackFast<240, 0>(out, in);
        break;
      case 1:
        UnpackFast<120, 0>(out, in);
        break;
      case 2:
        UnpackFast<60, 1>(out, in);
        break;
      case 3:
        UnpackFast<30, 2>(out, in);
        break;
      case 4:
        UnpackFast<20, 3>(out, in);
        break;
      case 5:
        UnpackFast<15, 4>(out, in);
        break;
      case 6:
        UnpackFast<12, 5>(out, in);
        break;
      case 7:
        UnpackFast<10, 6>(out, in);
        break;
      case 8:
        UnpackFast<8, 7>(out, in);
        break;
      case 9:
        UnpackFast<7, 8>(out, in);
        break;
      case 10:
        UnpackFast<6, 10>(out, in);
        break;
      case 11:
        UnpackFast<5, 12>(out, in);
        break;
      case 12:
        UnpackFast<4, 15>(out, in);
        break;
      case 13:
        UnpackFast<3, 20>(out, in);
        break;
      case 14:
        UnpackFast<2, 30>(out, in);
        break;
      case 15:
        UnpackFast<1, 60>(out, in);
        break;
      default:
        break;
    }
  }
  while (end > out) {
    switch (GetSelectorNum(in)) {
      case 0:
        UnpackCareful<0>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 240), out, in);
        break;
      case 1:
        UnpackCareful<0>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 120), out, in);
        break;
      case 2:
        UnpackCareful<1>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 60), out, in);
        break;
      case 3:
        UnpackCareful<2>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 30), out, in);
        break;
      case 4:
        UnpackCareful<3>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 20), out, in);
        break;
      case 5:
        UnpackCareful<4>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 15), out, in);
        break;
      case 6:
        UnpackCareful<5>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 12), out, in);
        break;
      case 7:
        UnpackCareful<6>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 10), out, in);
        break;
      case 8:
        UnpackCareful<7>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 8), out, in);
        break;
      case 9:
        UnpackCareful<8>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 7), out, in);
        break;
      case 10:
        UnpackCareful<10>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 6), out, in);
        break;
      case 11:
        UnpackCareful<12>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 5), out, in);
        break;
      case 12:
        UnpackCareful<15>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 4), out, in);
        break;
      case 13:
        UnpackCareful<20>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 3), out, in);
        break;
      case 14:
        UnpackCareful<30>(std::min<uint32_t>(static_cast<uint32_t>(end - out), 2), out, in);
        break;
      case 15:
        UnpackCareful<60>(1, out, in);
        break;
      default:
        break;
    }
  }

  // ASSERT(out < end + 240, out - end);
  return out - initout;
}

template <typename T>
void ZigZagEncode(T *input, size_t length) {
  T shift = (sizeof(T) * 8) - 1;  // sizeof * 8 = # of bits
  for (size_t i = 0; i < length; i++) {
    input[i] = (input[i] << 1LL) ^ (input[i] >> shift);
  }
  return;
}

template <typename T>
void ZigZagDecode(T *input, size_t length) {
  for (size_t i = 0; i < length; i++) {
    input[i] = (input[i] >> 1LL) ^ -(input[i] & 1LL);
  }
  return;
}
