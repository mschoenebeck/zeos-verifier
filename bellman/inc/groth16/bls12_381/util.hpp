#pragma once

#include <tuple>

using namespace std;

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/// Compute a + b + carry, returning the result and the new carry over.
tuple<uint64_t, uint64_t> adc(uint64_t a, uint64_t b, uint64_t carry);

/// Compute a - (b + borrow), returning the result and the new borrow.
tuple<uint64_t, uint64_t> sbb(uint64_t a, uint64_t b, uint64_t borrow);

/// Compute a + (b * c) + carry, returning the result and the new carry over.
tuple<uint64_t, uint64_t> mac(uint64_t a, uint64_t b, uint64_t c, uint64_t carry);
