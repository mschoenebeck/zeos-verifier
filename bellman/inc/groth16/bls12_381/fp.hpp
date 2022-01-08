#pragma once

#include <vector>
#include <tuple>
#include "subtle.hpp"
#include "util.hpp"
#include "json_struct.h"

using namespace std;

// The internal representation of this type is six 64-bit unsigned
// integers in little-endian order. `Fp` values are always in
// Montgomery form; i.e., Scalar(a) = aR mod p, with R = 2^384.
class Fp
{
    public:

        // constants
        /// p = 4002409555221667393417789825735904156556882819939007885332058136124031650490837864442687629129015664037894272559787
        static const vector<uint64_t> MODULUS;

        /// INV = -(p^{-1} mod 2^64) mod 2^64
        static const uint64_t INV;

        /// R = 2^384 mod p
        static const Fp R;

        /// R2 = 2^(384*2) mod p
        static const Fp R2;

        /// R3 = 2^(384*3) mod p
        static const Fp R3;

        static const Fp ZERO;

        // members
        vector<uint64_t> data;
        JS_OBJ(data);

        Fp();
        Fp(vector<uint64_t> data);

        Choice ct_eq(const Fp& other) const;

        static Fp conditional_select(const Fp& a, const Fp& b, const Choice& choice);

        static Fp zero();

        static Fp one();

        Choice is_zero() const;

        static Fp from_raw_unchecked(vector<uint64_t> v);

        static Fp montgomery_reduce(
                uint64_t t0,
                uint64_t t1,
                uint64_t t2,
                uint64_t t3,
                uint64_t t4,
                uint64_t t5,
                uint64_t t6,
                uint64_t t7,
                uint64_t t8,
                uint64_t t9,
                uint64_t t10,
                uint64_t t11);

        Fp pow_vartime(vector<uint64_t> by) const;

        tuple<Fp, Choice> invert() const;

        Fp subtract_p() const;

        Fp add(const Fp& rhs) const;

        Fp neg() const;

        Fp sub(const Fp& rhs) const;

        Fp mul(const Fp& rhs) const;

        Fp square() const;

        Fp operator-() const;

        Fp operator + (const Fp& rhs) const;

        Fp operator - (const Fp& rhs) const;

        Fp operator * (const Fp& rhs) const;
};
