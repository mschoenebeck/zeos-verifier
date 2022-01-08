
#include "groth16/bls12_381/util.hpp"

/// Compute a + b + carry, returning the result and the new carry over.
tuple<uint64_t, uint64_t> adc(uint64_t a, uint64_t b, uint64_t carry)
{
    uint128_t ret = (uint128_t)a + (uint128_t)b + (uint128_t)carry;
    return tuple<uint64_t, uint64_t>{(uint64_t)ret, (uint64_t)(ret >> 64)};
}

/// Compute a - (b + borrow), returning the result and the new borrow.
tuple<uint64_t, uint64_t> sbb(uint64_t a, uint64_t b, uint64_t borrow)
{
    uint128_t ret = (uint128_t)a - ((uint128_t)b + (uint128_t)(borrow >> 63));
    return tuple<uint64_t, uint64_t>{(uint64_t)ret, (uint64_t)(ret >> 64)};
}

/// Compute a + (b * c) + carry, returning the result and the new carry over.
tuple<uint64_t, uint64_t> mac(uint64_t a, uint64_t b, uint64_t c, uint64_t carry)
{
    uint128_t ret = (uint128_t)a + ((uint128_t)b * (uint128_t)c) + (uint128_t)carry;
    return tuple<uint64_t, uint64_t>{(uint64_t)ret, (uint64_t)(ret >> 64)};
}
