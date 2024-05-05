#pragma once
#include <stdint.h>

template <typename T>
size_t Simple8bEncode(T *input, size_t inputLength, uint64_t *out);

template <typename T>
const size_t Simple8bDecode(uint64_t *input, size_t uncompressedLength, T *out);

template <typename T>
void DeltaEncode(T *input, size_t length);
template <typename T>
void DeltaDecode(T *input, size_t length);
template <typename T>
void ZigZagEncode(T *input, size_t length);
template <typename T>
void ZigZagDecode(T *input, size_t length);

#include "simple8b.inl"