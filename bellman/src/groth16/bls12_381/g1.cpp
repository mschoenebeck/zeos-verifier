
#include "groth16/bls12_381/g1.hpp"

// G1Affine implementations

G1Affine::G1Affine() : x(Fp()), y(Fp()), infinity(Choice())
{
}

G1Affine::G1Affine(const Fp& x, const Fp& y, const Choice& infinity) : x(x), y(y), infinity(infinity)
{
}

G1Affine G1Affine::from(const G1Projective& p)
{
    Fp zinv;
    Choice c;
    tie(zinv, c) = p.z.invert();
    zinv = Fp::conditional_select(Fp::zero(), zinv, c);

    Fp x = p.x * zinv;
    Fp y = p.y * zinv;

    G1Affine tmp(x, y, Choice(0));

    return conditional_select(tmp, identity(), zinv.is_zero());
}

G1Affine G1Affine::conditional_select(const G1Affine& a, const G1Affine& b, const Choice& choice)
{
    return G1Affine(Fp::conditional_select(a.x, b.x, choice),
                    Fp::conditional_select(a.y, b.y, choice),
                    Choice::conditional_select(a.infinity, b.infinity, choice));
}

G1Affine G1Affine::generator()
{
    return G1Affine(
        Fp::from_raw_unchecked({
            0x5cb3'8790'fd53'0c16,
            0x7817'fc67'9976'fff5,
            0x154f'95c7'143b'a1c1,
            0xf0ae'6acd'f3d0'e747,
            0xedce'6ecc'21db'f440,
            0x1201'7741'9e0b'fb75,
        }),
        Fp::from_raw_unchecked({
            0xbaac'93d5'0ce7'2271,
            0x8c22'631a'7918'fd8e,
            0xdd59'5f13'5707'25ce,
            0x51ac'5829'5040'5194,
            0x0e1c'8c3f'ad00'59c0,
            0x0bbc'3efc'5008'a26a,
        }),
        Choice(0));
}

G1Affine G1Affine::identity()
{
    return G1Affine(Fp::zero(),
                    Fp::one(),
                    Choice(1));
}

Choice G1Affine::is_identity() const
{
    return this->infinity;
}

G1Projective G1Affine::to_curve() const
{
    return G1Projective(this->x,
                        this->y,
                        Fp::conditional_select(Fp::one(), Fp::zero(), this->infinity));
}

// G1Projective implementations

G1Projective::G1Projective() : x(Fp()), y(Fp()), z(Fp())
{
}

G1Projective::G1Projective(const Fp& x, const Fp& y, const Fp& z) : x(x), y(y), z(z)
{
}

G1Affine G1Projective::to_affine() const
{
    return G1Affine::from(*this);
}

G1Projective G1Projective::conditional_select(const G1Projective& a, const G1Projective& b, const Choice& choice)
{
    return G1Projective(Fp::conditional_select(a.x, b.x, choice),
                        Fp::conditional_select(a.y, b.y, choice),
                        Fp::conditional_select(a.z, b.z, choice));
}

G1Projective G1Projective::identity()
{
    return G1Projective(Fp::zero(),
                        Fp::one(),
                        Fp::zero());
}

Choice G1Projective::is_identity() const
{
    return this->z.is_zero();
}

G1Projective G1Projective::dbl() const
{
    // Algorithm 9, https://eprint.iacr.org/2015/1060.pdf

    Fp t0 = this->y.square();
    Fp z3 = t0 + t0;
       z3 = z3 + z3;
       z3 = z3 + z3;
    Fp t1 = this->y * this->z;
    Fp t2 = this->z.square();
       t2 = mul_by_3b(t2);
    Fp x3 = t2 * z3;
    Fp y3 = t0 + t2;
       z3 = t1 * z3;
       t1 = t2 + t2;
       t2 = t1 + t2;
       t0 = t0 - t2;
       y3 = t0 * y3;
       y3 = x3 + y3;
       t1 = this->x * this->y;
       x3 = t0 * t1;
       x3 = x3 + x3;

    return G1Projective::conditional_select(G1Projective(x3, y3, z3), G1Projective::identity(), this->is_identity());
}

G1Projective G1Projective::add(const G1Projective& rhs) const
{
    // Algorithm 7, https://eprint.iacr.org/2015/1060.pdf

    Fp t0 = this->x * rhs.x;
    Fp t1 = this->y * rhs.y;
    Fp t2 = this->z * rhs.z;
    Fp t3 = this->x + this->y;
    Fp t4 = rhs.x + rhs.y;
       t3 = t3 * t4;
       t4 = t0 + t1;
       t3 = t3 - t4;
       t4 = this->y + this->z;
    Fp x3 = rhs.y + rhs.z;
       t4 = t4 * x3;
       x3 = t1 + t2;
       t4 = t4 - x3;
       x3 = this->x + this->z;
    Fp y3 = rhs.x + rhs.z;
       x3 = x3 * y3;
       y3 = t0 + t2;
       y3 = x3 - y3;
       x3 = t0 + t0;
       t0 = x3 + t0;
       t2 = mul_by_3b(t2);
    Fp z3 = t1 + t2;
       t1 = t1 - t2;
       y3 = mul_by_3b(y3);
       x3 = t4 * y3;
       t2 = t3 * t1;
       x3 = t2 - x3;
       y3 = y3 * t0;
       t1 = t1 * z3;
       y3 = t1 + y3;
       t0 = t0 * t3;
       z3 = z3 * t4;
       z3 = z3 + t0;

    return G1Projective(x3, y3, z3);
}

G1Projective G1Projective::mul(const Scalar& rhs) const
{
    G1Projective acc = G1Projective::identity();
    vector<uint8_t> by = rhs.to_bytes();

    // This is a simple double-and-add implementation of point
    // multiplication, moving from most significant to least
    // significant bit of the scalar.
    //
    // We skip the leading bit because it's always unset for Fq
    // elements.
    for(int i = 31; i >= 0; i--)
    {
        for(int j = i==31?6:7; j >= 0; j--)
        {
            acc = acc.dbl();
            acc = conditional_select(acc, acc + *this, Choice((by[i] >> j) & 1));
        }
    }

    return acc;
}

G1Projective G1Projective::operator + (const G1Projective& rhs) const
{
    return this->add(rhs);
}

G1Projective G1Projective::operator * (const Scalar& rhs) const
{
    return this->mul(rhs);
}



Fp mul_by_3b(const Fp& a)
{
    Fp b = a + a;       // 2
       b = b + b;       // 4
       b = b + b + b;   // 12

    return b;
}