
#include "groth16/bls12_381/fp12.hpp"

Fp12::Fp12() : c0(Fp6::zero()), c1(Fp6::zero())
{
}

Fp12::Fp12(const Fp6& c0, const Fp6& c1) : c0(c0), c1(c1)
{
}

Fp12 Fp12::from(const Fp& f)
{
    return Fp12(Fp6::from(f),
                Fp6::zero());
}

Fp12 Fp12::from(const Fp2& f)
{
    return Fp12(Fp6::from(f),
                Fp6::zero());
}

Fp12 Fp12::from(const Fp6& f)
{
    return Fp12(f, Fp6::zero());
}

Choice Fp12::ct_eq(const Fp12& other) const
{
    return this->c0.ct_eq(other.c0) & this->c1.ct_eq(other.c1);
}

Fp12 Fp12::conditional_select(const Fp12& a, const Fp12& b, const Choice& choice)
{
    return Fp12(Fp6::conditional_select(a.c0, b.c0, choice),
                Fp6::conditional_select(a.c1, b.c1, choice));
}

Fp12 Fp12::zero()
{
    return Fp12(Fp6::zero(),
                Fp6::zero());
}

Fp12 Fp12::one()
{
    return Fp12(Fp6::one(),
                Fp6::zero());
}

Fp12 Fp12::mul_by_014(const Fp2& c0, const Fp2& c1, const Fp2& c4) const
{
    Fp6 aa  = this->c0.mul_by_01(c0, c1);
    Fp6 bb  = this->c1.mul_by_1(c4);
    Fp2 o   = c1 + c4;
    Fp6 cc1 = this->c1 + this->c0;
        cc1 = cc1.mul_by_01(c0, o);
        cc1 = cc1 - aa - bb;
    Fp6 cc0 = bb;
        cc0 = cc0.mul_by_nonresidue();
        cc0 = cc0 + aa;

    return Fp12(cc0, cc1);
}

Fp12 Fp12::conjugate() const
{
    return Fp12(this->c0,
                -this->c1);
}

Fp12 Fp12::frobenius_map() const
{
    Fp6 c0 = this->c0.frobenius_map();
    Fp6 c1 = this->c1.frobenius_map();

    // c1 = c1 * (u + 1)^((p - 1) / 6)
    c1 = c1 * Fp6::from(Fp2(
            Fp::from_raw_unchecked({
                0x0708'9552'b319'd465,
                0xc669'5f92'b50a'8313,
                0x97e8'3ccc'd117'228f,
                0xa35b'aeca'b2dc'29ee,
                0x1ce3'93ea'5daa'ce4d,
                0x08f2'220f'b0fb'66eb,
            }),
            Fp::from_raw_unchecked({
                0xb2f6'6aad'4ce5'd646,
                0x5842'a06b'fc49'7cec,
                0xcf48'95d4'2599'd394,
                0xc11b'9cba'40a8'e8d0,
                0x2e38'13cb'e5a0'de89,
                0x110e'efda'8884'7faf,
            })));

    return Fp12(c0, c1);
}

tuple<Fp12, Choice> Fp12::invert() const
{
    Fp6 t;
    Choice choice;
    tie(t, choice) = (this->c0.square() - this->c1.square().mul_by_nonresidue()).invert();

    return tuple<Fp12, Choice>{Fp12(this->c0 * t, this->c1 * -t), choice};
}

Fp12 Fp12::square() const
{
    Fp6 ab   = this->c0 * this->c1;
    Fp6 c0c1 = this->c0 + this->c1;
    Fp6 c0   = this->c1.mul_by_nonresidue();
        c0   = c0 + this->c0;
        c0   = c0 * c0c1;
        c0   = c0 - ab;
    Fp6 c1   = ab + ab;
        c0   = c0 - ab.mul_by_nonresidue();

    return Fp12(c0, c1);
}

Fp12 Fp12::mul(const Fp12& rhs) const
{
    Fp6 aa = this->c0 * rhs.c0;
    Fp6 bb = this->c1 * rhs.c1;
    Fp6 o = rhs.c0 + rhs.c1;
    Fp6 c1 = this->c1 + this->c0;
        c1 = c1 * o;
        c1 = c1 - aa;
        c1 = c1 - bb;
    Fp6 c0 = bb.mul_by_nonresidue();
        c0 = c0 + aa;

    return Fp12(c0, c1);
}

Fp12 Fp12::operator * (const Fp12& rhs) const
{
    return this->mul(rhs);
}
