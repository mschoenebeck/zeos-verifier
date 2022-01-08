
#include "groth16/bls12_381/fp.hpp"

// constants
/// p = 4002409555221667393417789825735904156556882819939007885332058136124031650490837864442687629129015664037894272559787
const vector<uint64_t> Fp::MODULUS =
{
    0xb9fe'ffff'ffff'aaab,
    0x1eab'fffe'b153'ffff,
    0x6730'd2a0'f6b0'f624,
    0x6477'4b84'f385'12bf,
    0x4b1b'a7b6'434b'acd7,
    0x1a01'11ea'397f'e69a,
};

/// INV = -(p^{-1} mod 2^64) mod 2^64
const uint64_t Fp::INV = 0x89f3'fffc'fffc'fffd;

/// R = 2^384 mod p
const Fp Fp::R = Fp({
    0x7609'0000'0002'fffd,
    0xebf4'000b'c40c'0002,
    0x5f48'9857'53c7'58ba,
    0x77ce'5853'7052'5745,
    0x5c07'1a97'a256'ec6d,
    0x15f6'5ec3'fa80'e493,
});

/// R2 = 2^(384*2) mod p
const Fp Fp::R2 = Fp({
    0xf4df'1f34'1c34'1746,
    0x0a76'e6a6'09d1'04f1,
    0x8de5'476c'4c95'b6d5,
    0x67eb'88a9'939d'83c0,
    0x9a79'3e85'b519'952d,
    0x1198'8fe5'92ca'e3aa,
});

/// R3 = 2^(384*3) mod p
const Fp Fp::R3 = Fp({
    0xed48'ac6b'd94c'a1e0,
    0x315f'831e'03a7'adf8,
    0x9a53'352a'615e'29dd,
    0x34c0'4e5e'921e'1761,
    0x2512'd435'6572'4728,
    0x0aa6'3460'9175'5d4d,
});

const Fp Fp::ZERO = Fp({0, 0, 0, 0, 0, 0});

Fp::Fp() : data({0, 0, 0, 0, 0, 0})
{
}

Fp::Fp(vector<uint64_t> data) : data(data)
{
}

Choice Fp::ct_eq(const Fp& other) const
{
    return Choice::ct_eq(this->data[0], other.data[0]) &
           Choice::ct_eq(this->data[1], other.data[1]) &
           Choice::ct_eq(this->data[2], other.data[2]) &
           Choice::ct_eq(this->data[3], other.data[3]) &
           Choice::ct_eq(this->data[4], other.data[4]) &
           Choice::ct_eq(this->data[5], other.data[5]);
}

Fp Fp::conditional_select(const Fp& a, const Fp& b, const Choice& choice)
{
    return Fp({
        (uint64_t)(a.data[0] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[0] ^ b.data[0]))),
        (uint64_t)(a.data[1] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[1] ^ b.data[1]))),
        (uint64_t)(a.data[2] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[2] ^ b.data[2]))),
        (uint64_t)(a.data[3] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[3] ^ b.data[3]))),
        (uint64_t)(a.data[4] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[4] ^ b.data[4]))),
        (uint64_t)(a.data[5] ^ (-((int64_t)choice.unwrap_u8()) & (a.data[5] ^ b.data[5]))),
    });
}

Fp Fp::zero()
{
    return ZERO;
}

Fp Fp::one()
{
    return R;
}

Choice Fp::is_zero() const
{
    return this->ct_eq(zero());
}

Fp Fp::from_raw_unchecked(vector<uint64_t> v)
{
    return Fp(v);
}

Fp Fp::montgomery_reduce(
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
        uint64_t t11)
{
    // The Montgomery reduction here is based on Algorithm 14.32 in
    // Handbook of Applied Cryptography
    // <http://cacr.uwaterloo.ca/hac/about/chap14.pdf>.

    uint64_t _, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, carry;
    uint64_t k = t0 * INV;
    tie(_, carry) = mac(t0, k, MODULUS[0], 0);
    tie(r1, carry) = mac(t1, k, MODULUS[1], carry);
    tie(r2, carry) = mac(t2, k, MODULUS[2], carry);
    tie(r3, carry) = mac(t3, k, MODULUS[3], carry);
    tie(r4, carry) = mac(t4, k, MODULUS[4], carry);
    tie(r5, carry) = mac(t5, k, MODULUS[5], carry);
    tie(r6, r7) = adc(t6, 0, carry);

    k = r1 * INV;
    tie(_, carry) = mac(r1, k, MODULUS[0], 0);
    tie(r2, carry) = mac(r2, k, MODULUS[1], carry);
    tie(r3, carry) = mac(r3, k, MODULUS[2], carry);
    tie(r4, carry) = mac(r4, k, MODULUS[3], carry);
    tie(r5, carry) = mac(r5, k, MODULUS[4], carry);
    tie(r6, carry) = mac(r6, k, MODULUS[5], carry);
    tie(r7, r8) = adc(t7, r7, carry);

    k = r2 * INV;
    tie(_, carry) = mac(r2, k, MODULUS[0], 0);
    tie(r3, carry) = mac(r3, k, MODULUS[1], carry);
    tie(r4, carry) = mac(r4, k, MODULUS[2], carry);
    tie(r5, carry) = mac(r5, k, MODULUS[3], carry);
    tie(r6, carry) = mac(r6, k, MODULUS[4], carry);
    tie(r7, carry) = mac(r7, k, MODULUS[5], carry);
    tie(r8, r9) = adc(t8, r8, carry);

    k = r3 * INV;
    tie(_, carry) = mac(r3, k, MODULUS[0], 0);
    tie(r4, carry) = mac(r4, k, MODULUS[1], carry);
    tie(r5, carry) = mac(r5, k, MODULUS[2], carry);
    tie(r6, carry) = mac(r6, k, MODULUS[3], carry);
    tie(r7, carry) = mac(r7, k, MODULUS[4], carry);
    tie(r8, carry) = mac(r8, k, MODULUS[5], carry);
    tie(r9, r10) = adc(t9, r9, carry);

    k = r4 * INV;
    tie(_, carry) = mac(r4, k, MODULUS[0], 0);
    tie(r5, carry) = mac(r5, k, MODULUS[1], carry);
    tie(r6, carry) = mac(r6, k, MODULUS[2], carry);
    tie(r7, carry) = mac(r7, k, MODULUS[3], carry);
    tie(r8, carry) = mac(r8, k, MODULUS[4], carry);
    tie(r9, carry) = mac(r9, k, MODULUS[5], carry);
    tie(r10, r11) = adc(t10, r10, carry);

    k = r5 * INV;
    tie(_, carry) = mac(r5, k, MODULUS[0], 0);
    tie(r6, carry) = mac(r6, k, MODULUS[1], carry);
    tie(r7, carry) = mac(r7, k, MODULUS[2], carry);
    tie(r8, carry) = mac(r8, k, MODULUS[3], carry);
    tie(r9, carry) = mac(r9, k, MODULUS[4], carry);
    tie(r10, carry) = mac(r10, k, MODULUS[5], carry);
    tie(r11, _) = adc(t11, r11, carry);

    // Attempt to subtract the modulus, to ensure the value
    // is smaller than the modulus.
    return Fp({r6, r7, r8, r9, r10, r11}).subtract_p();
}

Fp Fp::pow_vartime(vector<uint64_t> by) const
{
    Fp res = one();
    for(int e = 5; e >= 0; e--)
    {
        for(int i = 63; i >= 0; i--)
        {
            res = res.square();

            if(((by[e] >> i) & 1) == 1)
            {
                res = res * *this;
            }
        }
    }
    return res;
}

tuple<Fp, Choice> Fp::invert() const
{
    // Exponentiate by p - 2
    Fp t = this->pow_vartime({
        0xb9fe'ffff'ffff'aaa9,
        0x1eab'fffe'b153'ffff,
        0x6730'd2a0'f6b0'f624,
        0x6477'4b84'f385'12bf,
        0x4b1b'a7b6'434b'acd7,
        0x1a01'11ea'397f'e69a,
    });

    return tuple<Fp, Choice>{t, !this->is_zero()};
}

Fp Fp::subtract_p() const
{
    uint64_t r0, r1, r2, r3, r4, r5, borrow;
    tie(r0, borrow) = sbb(this->data[0], MODULUS[0], 0);
    tie(r1, borrow) = sbb(this->data[1], MODULUS[1], borrow);
    tie(r2, borrow) = sbb(this->data[2], MODULUS[2], borrow);
    tie(r3, borrow) = sbb(this->data[3], MODULUS[3], borrow);
    tie(r4, borrow) = sbb(this->data[4], MODULUS[4], borrow);
    tie(r5, borrow) = sbb(this->data[5], MODULUS[5], borrow);

    // If underflow occurred on the final limb, borrow = 0xfff...fff, otherwise
    // borrow = 0x000...000. Thus, we use it as a mask!
    r0 = (this->data[0] & borrow) | (r0 & ~borrow);
    r1 = (this->data[1] & borrow) | (r1 & ~borrow);
    r2 = (this->data[2] & borrow) | (r2 & ~borrow);
    r3 = (this->data[3] & borrow) | (r3 & ~borrow);
    r4 = (this->data[4] & borrow) | (r4 & ~borrow);
    r5 = (this->data[5] & borrow) | (r5 & ~borrow);

    return Fp({r0, r1, r2, r3, r4, r5});
}

Fp Fp::add(const Fp& rhs) const
{
    uint64_t d0, d1, d2, d3, d4, d5, carry, _;
    tie(d0, carry) = adc(this->data[0], rhs.data[0], 0);
    tie(d1, carry) = adc(this->data[1], rhs.data[1], carry);
    tie(d2, carry) = adc(this->data[2], rhs.data[2], carry);
    tie(d3, carry) = adc(this->data[3], rhs.data[3], carry);
    tie(d4, carry) = adc(this->data[4], rhs.data[4], carry);
    tie(d5, _)     = adc(this->data[5], rhs.data[5], carry);

    // Attempt to subtract the modulus, to ensure the value
    // is smaller than the modulus.
    return Fp({d0, d1, d2, d3, d4, d5}).subtract_p();
}

Fp Fp::neg() const
{
    uint64_t d0, d1, d2, d3, d4, d5, borrow, _;
    tie(d0, borrow) = sbb(MODULUS[0], data[0], 0);
    tie(d1, borrow) = sbb(MODULUS[1], data[1], borrow);
    tie(d2, borrow) = sbb(MODULUS[2], data[2], borrow);
    tie(d3, borrow) = sbb(MODULUS[3], data[3], borrow);
    tie(d4, borrow) = sbb(MODULUS[4], data[4], borrow);
    tie(d5, _)      = sbb(MODULUS[5], data[5], borrow);

    // Let's use a mask if `self` was zero, which would mean
    // the result of the subtraction is p.
    uint64_t mask = ((data[0] | data[1] | data[2] | data[3] | data[4] | data[5]) == 0) - 1;

    return Fp({
        d0 & mask,
        d1 & mask,
        d2 & mask,
        d3 & mask,
        d4 & mask,
        d5 & mask,
    });
}

Fp Fp::sub(const Fp& rhs) const
{
    return rhs.neg().add(*this);
}

Fp Fp::mul(const Fp& rhs) const
{
    uint64_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, carry;
    tie(t0, carry) = mac(0, this->data[0], rhs.data[0], 0);
    tie(t1, carry) = mac(0, this->data[0], rhs.data[1], carry);
    tie(t2, carry) = mac(0, this->data[0], rhs.data[2], carry);
    tie(t3, carry) = mac(0, this->data[0], rhs.data[3], carry);
    tie(t4, carry) = mac(0, this->data[0], rhs.data[4], carry);
    tie(t5, t6)    = mac(0, this->data[0], rhs.data[5], carry);

    tie(t1, carry) = mac(t1, this->data[1], rhs.data[0], 0);
    tie(t2, carry) = mac(t2, this->data[1], rhs.data[1], carry);
    tie(t3, carry) = mac(t3, this->data[1], rhs.data[2], carry);
    tie(t4, carry) = mac(t4, this->data[1], rhs.data[3], carry);
    tie(t5, carry) = mac(t5, this->data[1], rhs.data[4], carry);
    tie(t6, t7)    = mac(t6, this->data[1], rhs.data[5], carry);

    tie(t2, carry) = mac(t2, this->data[2], rhs.data[0], 0);
    tie(t3, carry) = mac(t3, this->data[2], rhs.data[1], carry);
    tie(t4, carry) = mac(t4, this->data[2], rhs.data[2], carry);
    tie(t5, carry) = mac(t5, this->data[2], rhs.data[3], carry);
    tie(t6, carry) = mac(t6, this->data[2], rhs.data[4], carry);
    tie(t7, t8)    = mac(t7, this->data[2], rhs.data[5], carry);

    tie(t3, carry) = mac(t3, this->data[3], rhs.data[0], 0);
    tie(t4, carry) = mac(t4, this->data[3], rhs.data[1], carry);
    tie(t5, carry) = mac(t5, this->data[3], rhs.data[2], carry);
    tie(t6, carry) = mac(t6, this->data[3], rhs.data[3], carry);
    tie(t7, carry) = mac(t7, this->data[3], rhs.data[4], carry);
    tie(t8, t9)    = mac(t8, this->data[3], rhs.data[5], carry);

    tie(t4, carry) = mac(t4, this->data[4], rhs.data[0], 0);
    tie(t5, carry) = mac(t5, this->data[4], rhs.data[1], carry);
    tie(t6, carry) = mac(t6, this->data[4], rhs.data[2], carry);
    tie(t7, carry) = mac(t7, this->data[4], rhs.data[3], carry);
    tie(t8, carry) = mac(t8, this->data[4], rhs.data[4], carry);
    tie(t9, t10)   = mac(t9, this->data[4], rhs.data[5], carry);

    tie(t5, carry) = mac(t5, this->data[5], rhs.data[0], 0);
    tie(t6, carry) = mac(t6, this->data[5], rhs.data[1], carry);
    tie(t7, carry) = mac(t7, this->data[5], rhs.data[2], carry);
    tie(t8, carry) = mac(t8, this->data[5], rhs.data[3], carry);
    tie(t9, carry) = mac(t9, this->data[5], rhs.data[4], carry);
    tie(t10, t11)  = mac(t10, this->data[5], rhs.data[5], carry);

    return montgomery_reduce(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
}

Fp Fp::square() const
{
    uint64_t _, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, carry;
    tie(t1, carry) = mac(0, this->data[0], this->data[1], 0);
    tie(t2, carry) = mac(0, this->data[0], this->data[2], carry);
    tie(t3, carry) = mac(0, this->data[0], this->data[3], carry);
    tie(t4, carry) = mac(0, this->data[0], this->data[4], carry);
    tie(t5, t6)    = mac(0, this->data[0], this->data[5], carry);

    tie(t3, carry) = mac(t3, this->data[1], this->data[2], 0);
    tie(t4, carry) = mac(t4, this->data[1], this->data[3], carry);
    tie(t5, carry) = mac(t5, this->data[1], this->data[4], carry);
    tie(t6, t7)    = mac(t6, this->data[1], this->data[5], carry);

    tie(t5, carry) = mac(t5, this->data[2], this->data[3], 0);
    tie(t6, carry) = mac(t6, this->data[2], this->data[4], carry);
    tie(t7, t8)    = mac(t7, this->data[2], this->data[5], carry);

    tie(t7, carry) = mac(t7, this->data[3], this->data[4], 0);
    tie(t8, t9)    = mac(t8, this->data[3], this->data[5], carry);

    tie(t9, t10)   = mac(t9, this->data[4], this->data[5], 0);

    t11 = t10 >> 63;
    t10 = (t10 << 1) | (t9 >> 63);
    t9 = (t9 << 1) | (t8 >> 63);
    t8 = (t8 << 1) | (t7 >> 63);
    t7 = (t7 << 1) | (t6 >> 63);
    t6 = (t6 << 1) | (t5 >> 63);
    t5 = (t5 << 1) | (t4 >> 63);
    t4 = (t4 << 1) | (t3 >> 63);
    t3 = (t3 << 1) | (t2 >> 63);
    t2 = (t2 << 1) | (t1 >> 63);
    t1 = t1 << 1;

    tie(t0, carry) = mac(0, this->data[0], this->data[0], 0);
    tie(t1, carry) = adc(t1, 0, carry);
    tie(t2, carry) = mac(t2, this->data[1], this->data[1], carry);
    tie(t3, carry) = adc(t3, 0, carry);
    tie(t4, carry) = mac(t4, this->data[2], this->data[2], carry);
    tie(t5, carry) = adc(t5, 0, carry);
    tie(t6, carry) = mac(t6, this->data[3], this->data[3], carry);
    tie(t7, carry) = adc(t7, 0, carry);
    tie(t8, carry) = mac(t8, this->data[4], this->data[4], carry);
    tie(t9, carry) = adc(t9, 0, carry);
    tie(t10, carry) = mac(t10, this->data[5], this->data[5], carry);
    tie(t11, _)    = adc(t11, 0, carry);

    return montgomery_reduce(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
}

Fp Fp::operator-() const
{
    return this->neg();
}

Fp Fp::operator + (const Fp& rhs) const
{
    return this->add(rhs);
}

Fp Fp::operator - (const Fp& rhs) const
{
    return this->sub(rhs);
}

Fp Fp::operator * (const Fp& rhs) const
{
    return this->mul(rhs);
}
