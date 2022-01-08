#pragma once

#include "fp.hpp"
#include "subtle.hpp"

class G1Projective;
class Scalar;

class G1Affine
{
    public:

        // members
        Fp x;
        Fp y;
        Choice infinity;
        JS_OBJ(x, y, infinity);

        G1Affine();
        G1Affine(const Fp& x, const Fp& y, const Choice& infinity);

        static G1Affine from(const G1Projective& p);

        static G1Affine conditional_select(const G1Affine& a, const G1Affine& b, const Choice& choice);

        static G1Affine generator();

        static G1Affine identity();

        Choice is_identity() const;

        G1Projective to_curve() const;
};

class G1Projective
{
    public:

        // members
        Fp x;
        Fp y;
        Fp z;
        JS_OBJ(x, y, z);

        G1Projective();
        G1Projective(const Fp& x, const Fp& y, const Fp& z);

        G1Affine to_affine() const;

        static G1Projective conditional_select(const G1Projective& a, const G1Projective& b, const Choice& choice);

        static G1Projective identity();

        Choice is_identity() const;

        G1Projective dbl() const;

        G1Projective add(const G1Projective& rhs) const;

        G1Projective mul(const Scalar& rhs) const;

        G1Projective operator + (const G1Projective& rhs) const;

        G1Projective operator * (const Scalar& rhs) const;
};



class Scalar
{
    public:

        // constants
        /// INV = -(q^{-1} mod 2^64) mod 2^64
        static const uint64_t INV;

        /// Constant representing the modulus
        /// q = 0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001
        static const Scalar MODULUS;

        // members
        vector<uint64_t> data;
        JS_OBJ(data);

        Scalar();
        Scalar(const vector<uint64_t>& data);

        /// Converts an element of `Scalar` into a byte representation in
        /// little-endian byte order.
        vector<uint8_t> to_bytes() const;

        static Scalar montgomery_reduce(const uint64_t& r0,
                                        const uint64_t& r1,
                                        const uint64_t& r2,
                                        const uint64_t& r3,
                                        const uint64_t& r4,
                                        const uint64_t& r5,
                                        const uint64_t& r6,
                                        const uint64_t& r7);

        Scalar sub(const Scalar& rhs) const;

        Scalar operator - (const Scalar& rhs) const;
};

Fp mul_by_3b(const Fp& a);
