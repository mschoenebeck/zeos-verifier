
#include "groth16/bls12_381/fp6.hpp"

Fp6::Fp6() : c0(Fp2::zero()), c1(Fp2::zero()), c2(Fp2::zero())
{
}

Fp6::Fp6(const Fp2& c0, const Fp2& c1, const Fp2& c2) : c0(c0), c1(c1), c2(c2)
{
}

Fp6 Fp6::from(const Fp& f)
{
    return Fp6(Fp2::from(f),
               Fp2::zero(),
               Fp2::zero());
}

Fp6 Fp6::from(const Fp2& f)
{
    return Fp6(f,
               Fp2::zero(),
               Fp2::zero());
}

Choice Fp6::ct_eq(const Fp6& other) const
{
    return this->c0.ct_eq(other.c0) & this->c1.ct_eq(other.c1) & this->c2.ct_eq(other.c2);
}

Fp6 Fp6::conditional_select(const Fp6& a, const Fp6& b, const Choice& choice)
{
    return Fp6(Fp2::conditional_select(a.c0, b.c0, choice),
               Fp2::conditional_select(a.c1, b.c1, choice),
               Fp2::conditional_select(a.c2, b.c2, choice));
}

Fp6 Fp6::zero()
{
    return Fp6();
}

Fp6 Fp6::one()
{
    return Fp6(Fp2::one(), Fp2::zero(), Fp2::zero());
}

Fp6 Fp6::frobenius_map() const
{
    Fp2 c0 = this->c0.frobenius_map();
    Fp2 c1 = this->c1.frobenius_map();
    Fp2 c2 = this->c2.frobenius_map();

    // c1 = c1 * (u + 1)^((p - 1) / 3)
    c1 = c1 * Fp2(Fp::zero(),
                  Fp::from_raw_unchecked({
                      0xcd03'c9e4'8671'f071,
                      0x5dab'2246'1fcd'a5d2,
                      0x5870'42af'd385'1b95,
                      0x8eb6'0ebe'01ba'cb9e,
                      0x03f9'7d6e'83d0'50d2,
                      0x18f0'2065'5463'8741,
                  }));

    // c2 = c2 * (u + 1)^((2p - 2) / 3)
    c2 = c2* Fp2(Fp::from_raw_unchecked({
                     0x890d'c9e4'8675'45c3,
                     0x2af3'2253'3285'a5d5,
                     0x5088'0866'309b'7e2c,
                     0xa20d'1b8c'7e88'1024,
                     0x14e4'f04f'e2db'9068,
                     0x14e5'6d3f'1564'853a,
                 }),
                 Fp::zero());

    return Fp6(c0, c1, c2);
}

Fp6 Fp6::mul_by_1(const Fp2& c1) const
{
    Fp2 b_b = this->c1 * c1;

    Fp2 t1 = (this->c1 + this->c2) * c1 - b_b;
        t1 = t1.mul_by_nonresidue();

    Fp2 t2 = (this->c0 + this->c1) * c1 - b_b;

    return Fp6(t1, t2, b_b);
}

Fp6 Fp6::mul_by_01(const Fp2& c0,const Fp2& c1) const
{
    Fp2 a_a = this->c0 * c0;
    Fp2 b_b = this->c1 * c1;

    Fp2 t1 = (this->c1 + this->c2) * c1 - b_b;
        t1 = t1.mul_by_nonresidue() + a_a;

    Fp2 t2 = (c0 + c1) * (this->c0 + this->c1) - a_a - b_b;

    Fp2 t3 = (this->c0 + this->c2) * c0 - a_a + b_b;

    return Fp6(t1, t2, t3);
}

Fp6 Fp6::mul_by_nonresidue() const
{
    // Given a + bv + cv^2, this produces
    //     av + bv^2 + cv^3
    // but because v^3 = u + 1, we have
    //     c(u + 1) + av + v^2

    return Fp6(this->c2.mul_by_nonresidue(),
               this->c0,
               this->c1);
}

tuple<Fp6, Choice> Fp6::invert() const
{
    Fp2 c0 = (this->c1 * this->c2).mul_by_nonresidue();
        c0 = this->c0.square() - c0;

    Fp2 c1 = this->c2.square().mul_by_nonresidue();
        c1 = c1 - (this->c0 * this->c1);

    Fp2 c2 = this->c1.square();
        c2 = c2 - (this->c0 * this->c2);

    Fp2 tmp = ((this->c1 * c2) + (this->c2 * c1)).mul_by_nonresidue();
        tmp = tmp + (this->c0 * c0);

    Fp2 t;
    Choice choice;
    tie(t, choice) = tmp.invert();
    return tuple<Fp6, Choice>{Fp6(t * c0, t * c1, t * c2), choice};
}

Fp6 Fp6::square() const
{
    Fp2 s0 = this->c0.square();
    Fp2 ab = this->c0 * this->c1;
    Fp2 s1 = ab + ab;
    Fp2 s2 = (this->c0 - this->c1 + this->c2).square();
    Fp2 bc = this->c1 * this->c2;
    Fp2 s3 = bc + bc;
    Fp2 s4 = this->c2.square();

    return Fp6(s3.mul_by_nonresidue() + s0,
               s4.mul_by_nonresidue() + s1,
               s1 + s2 + s3 - s0 - s4);
}

Fp6 Fp6::add(const Fp6& rhs) const
{
    return Fp6(this->c0 + rhs.c0,
               this->c1 + rhs.c1,
               this->c2 + rhs.c2);
}

Fp6 Fp6::neg() const
{
    return Fp6(-this->c0,
               -this->c1,
               -this->c2);
}

Fp6 Fp6::sub(const Fp6& rhs) const
{
    return Fp6(this->c0 - rhs.c0,
               this->c1 - rhs.c1,
               this->c2 - rhs.c2);
}

Fp6 Fp6::mul(const Fp6& rhs) const
{
    Fp2 aa = this->c0 * rhs.c0;
    Fp2 bb = this->c1 * rhs.c1;
    Fp2 cc = this->c2 * rhs.c2;

    Fp2 t1 = rhs.c1 + rhs.c2;
    Fp2 tmp = this->c1 + this->c2;
        t1 = t1 * tmp;
        t1 = t1 - bb;
        t1 = t1 - cc;
        t1 = t1.mul_by_nonresidue();
        t1 = t1 + aa;

    Fp2 t3 = rhs.c0 + rhs.c2;
        tmp = this->c0 + this->c2;
        t3 = t3 * tmp;
        t3 = t3 - aa;
        t3 = t3 + bb;
        t3 = t3 - cc;

    Fp2 t2 = rhs.c0 + rhs.c1;
        tmp = this->c0 + this->c1;
        t2 = t2 * tmp;
        t2 = t2 - aa;
        t2 = t2 - bb;
        cc = cc.mul_by_nonresidue();
        t2 = t2 + cc;

    return Fp6(t1, t2, t3);
}

Fp6 Fp6::operator-() const
{
    return this->neg();
}

Fp6 Fp6::operator + (const Fp6& rhs) const
{
    return this->add(rhs);
}

Fp6 Fp6::operator - (const Fp6& rhs) const
{
    return this->sub(rhs);
}

Fp6 Fp6::operator * (const Fp6& rhs) const
{
    return this->mul(rhs);
}
