#include "groth16/bls12_381/scalar.hpp"

// Scalar implementations

// constants
/// INV = -(q^{-1} mod 2^64) mod 2^64
const uint64_t Scalar::INV = 0xffff'fffe'ffff'ffff;

/// Constant representing the modulus
/// q = 0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001
const Scalar Scalar::MODULUS = Scalar(vector<uint64_t>{
    0xffff'ffff'0000'0001,
    0x53bd'a402'fffe'5bfe,
    0x3339'd808'09a1'd805,
    0x73ed'a753'299d'7d48
});

Scalar::Scalar() : data(vector<uint64_t>())
{
}

Scalar::Scalar(const vector<uint64_t>& data) : data(data)
{
}

vector<uint8_t> Scalar::to_bytes() const
{
    // Turn into canonical form by computing
    // (a.R) / R = a
    Scalar tmp = montgomery_reduce(this->data[0], this->data[1], this->data[2], this->data[3], 0, 0, 0, 0);

    vector<uint8_t> res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t* p = (uint8_t*)tmp.data.data();
    // mschoenebeck: the following code probably assumes little endian on the target machine. if target is big endian flip byte order here (UNTESTED)
    //res[0..8].copy_from_slice(&tmp.0[0].to_le_bytes());
    res[0] = p[0]; res[1] = p[1]; res[2] = p[2]; res[3] = p[3]; res[4] = p[4]; res[5] = p[5]; res[6] = p[6]; res[7] = p[7];
    //res[8..16].copy_from_slice(&tmp.0[1].to_le_bytes());
    res[8] = p[8]; res[9] = p[9]; res[10] = p[10]; res[11] = p[11]; res[12] = p[12]; res[13] = p[13]; res[14] = p[14]; res[15] = p[15];
    //res[16..24].copy_from_slice(&tmp.0[2].to_le_bytes());
    res[16] = p[16]; res[17] = p[17]; res[18] = p[18]; res[19] = p[19]; res[20] = p[20]; res[21] = p[21]; res[22] = p[22]; res[23] = p[23];
    //res[24..32].copy_from_slice(&tmp.0[3].to_le_bytes());
    res[24] = p[24]; res[25] = p[25]; res[26] = p[26]; res[27] = p[27]; res[28] = p[28]; res[29] = p[29]; res[30] = p[30]; res[31] = p[31];

    return res;
}

Scalar Scalar::montgomery_reduce(const uint64_t& r0,
                                 const uint64_t& r1,
                                 const uint64_t& r2,
                                 const uint64_t& r3,
                                 const uint64_t& r4,
                                 const uint64_t& r5,
                                 const uint64_t& r6,
                                 const uint64_t& r7)
{
    // The Montgomery reduction here is based on Algorithm 14.32 in
    // Handbook of Applied Cryptography
    // <http://cacr.uwaterloo.ca/hac/about/chap14.pdf>.

    uint64_t _, rr0 = r0, rr1 = r1, rr2 = r2, rr3 = r3, rr4 = r4, rr5 = r5, rr6 = r6, rr7 = r7, carry, carry2;
    uint64_t k = rr0 * INV;
    tie(_,   carry) = mac(rr0, k, MODULUS.data[0], 0);
    tie(rr1, carry) = mac(rr1, k, MODULUS.data[1], carry);
    tie(rr2, carry) = mac(rr2, k, MODULUS.data[2], carry);
    tie(rr3, carry) = mac(rr3, k, MODULUS.data[3], carry);
    tie(rr4, carry2) = adc(rr4, 0, carry);

    k = rr1 * INV;
    tie(_,   carry) = mac(rr1, k, MODULUS.data[0], 0);
    tie(rr2, carry) = mac(rr2, k, MODULUS.data[1], carry);
    tie(rr3, carry) = mac(rr3, k, MODULUS.data[2], carry);
    tie(rr4, carry) = mac(rr4, k, MODULUS.data[3], carry);
    tie(rr5, carry2) = adc(rr5, carry2, carry);

    k = rr2 * INV;
    tie(_,   carry) = mac(rr2, k, MODULUS.data[0], 0);
    tie(rr3, carry) = mac(rr3, k, MODULUS.data[1], carry);
    tie(rr4, carry) = mac(rr4, k, MODULUS.data[2], carry);
    tie(rr5, carry) = mac(rr5, k, MODULUS.data[3], carry);
    tie(rr6, carry2) = adc(rr6, carry2, carry);

    k = rr3 * INV;
    tie(_,   carry) = mac(rr3, k, MODULUS.data[0], 0);
    tie(rr4, carry) = mac(rr4, k, MODULUS.data[1], carry);
    tie(rr5, carry) = mac(rr5, k, MODULUS.data[2], carry);
    tie(rr6, carry) = mac(rr6, k, MODULUS.data[3], carry);
    tie(rr7, _) = adc(rr7, carry2, carry);

    // Result may be within MODULUS of the correct value
    return (Scalar(vector<uint64_t>{rr4, rr5, rr6, rr7})).sub(MODULUS);
}

Scalar Scalar::sub(const Scalar& rhs) const
{
    uint64_t _, d0, d1, d2, d3, borrow, carry;
    tie(d0, borrow) = sbb(this->data[0], rhs.data[0], 0);
    tie(d1, borrow) = sbb(this->data[1], rhs.data[1], borrow);
    tie(d2, borrow) = sbb(this->data[2], rhs.data[2], borrow);
    tie(d3, borrow) = sbb(this->data[3], rhs.data[3], borrow);

    // If underflow occurred on the final limb, borrow = 0xfff...fff, otherwise
    // borrow = 0x000...000. Thus, we use it as a mask to conditionally add the modulus.
    tie(d0, carry) = adc(d0, MODULUS.data[0] & borrow, 0);
    tie(d1, carry) = adc(d1, MODULUS.data[1] & borrow, carry);
    tie(d2, carry) = adc(d2, MODULUS.data[2] & borrow, carry);
    tie(d3, _)     = adc(d3, MODULUS.data[3] & borrow, carry);

    return Scalar(vector<uint64_t>{d0, d1, d2, d3});
}

Scalar Scalar::operator - (const Scalar& rhs) const
{
    return this->sub(rhs);
}
