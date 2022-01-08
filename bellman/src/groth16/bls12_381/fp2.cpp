
#include "groth16/bls12_381/fp2.hpp"

Fp2::Fp2() : c0(Fp::zero()), c1(Fp::zero())
{
}

Fp2::Fp2(const Fp& c0, const Fp& c1) : c0(c0), c1(c1)
{
}

Fp2 Fp2::from(const Fp& f)
{
    return Fp2(f,
               Fp::zero());
}

Choice Fp2::ct_eq(const Fp2& other) const
{
    return this->c0.ct_eq(other.c0) & this->c1.ct_eq(other.c1);
}

Fp2 Fp2::conditional_select(const Fp2& a, const Fp2& b, const Choice& choice)
{
    return Fp2(Fp::conditional_select(a.c0, b.c0, choice),
               Fp::conditional_select(a.c1, b.c1, choice));
}

Fp2 Fp2::frobenius_map() const
{
    // This is always just a conjugation. If you're curious why, here's
    // an article about it: https://alicebob.cryptoland.net/the-frobenius-endomorphism-with-finite-fields/
    return this->conjugate();
}

Fp2 Fp2::conjugate() const
{
    return Fp2(this->c0,
               -this->c1);
}

Fp2 Fp2::mul_by_nonresidue() const
{
    // Multiply a + bu by u + 1, getting
    // au + a + bu^2 + bu
    // and because u^2 = -1, we get
    // (a - b) + (a + b)u
    return Fp2(
        this->c0 - this->c1,
        this->c0 + this->c1);
}

Fp2 Fp2::zero()
{
    return Fp2(Fp::zero(),
               Fp::zero());
}

Fp2 Fp2::one()
{
    return Fp2(Fp::one(),
               Fp::zero());
}

tuple<Fp2, Choice> Fp2::invert() const
{
    // We wish to find the multiplicative inverse of a nonzero
    // element a + bu in Fp2. We leverage an identity
    //
    // (a + bu)(a - bu) = a^2 + b^2
    //
    // which holds because u^2 = -1. This can be rewritten as
    //
    // (a + bu)(a - bu)/(a^2 + b^2) = 1
    //
    // because a^2 + b^2 = 0 has no nonzero solutions for (a, b).
    // This gives that (a - bu)/(a^2 + b^2) is the inverse
    // of (a + bu). Importantly, this can be computing using
    // only a single inversion in Fp.

    Fp t;
    Choice choice;
    tie(t, choice) = (this->c0.square() + this->c1.square()).invert();
    return tuple<Fp2, Choice>{Fp2(this->c0 * t, this->c1 * -t), choice};
}

Fp2 Fp2::add(const Fp2& rhs) const
{
    return Fp2(this->c0.add(rhs.c0),
               this->c1.add(rhs.c1));
}

Fp2 Fp2::neg() const
{
    return Fp2(this->c0.neg(),
               this->c1.neg());
}

Fp2 Fp2::sub(const Fp2& rhs) const
{
    return Fp2(this->c0.sub(rhs.c0),
               this->c1.sub(rhs.c1));
}

Fp2 Fp2::mul(const Fp2& rhs) const
{
    // Karatsuba multiplication:
    //
    // v0  = a0 * b0
    // v1  = a1 * b1
    // c0 = v0 + \beta * v1
    // c1 = (a0 + a1) * (b0 + b1) - v0 - v1
    //
    // In BLS12-381's F_{p^2}, our \beta is -1 so we
    // can modify this formula. (Also, since we always
    // subtract v1, we can compute v1 = -a1 * b1.)
    //
    // v0  = a0 * b0
    // v1  = (-a1) * b1
    // c0 = v0 + v1
    // c1 = (a0 + a1) * (b0 + b1) - v0 + v1

    Fp v0 = this->c0.mul(rhs.c0);
    Fp v1 = (this->c1.neg()).mul(rhs.c1);
    Fp c0 = v0.add(v1);
    Fp c1 = (this->c0.add(this->c1)).mul(rhs.c0.add(rhs.c1));
       c1 = c1.sub(v0);
       c1 = c1.add(v1);

    return Fp2(c0, c1);
}

Fp2 Fp2::square() const
{
    // Complex squaring:
    //
    // v0  = c0 * c1
    // c0' = (c0 + c1) * (c0 + \beta*c1) - v0 - \beta * v0
    // c1' = 2 * v0
    //
    // In BLS12-381's F_{p^2}, our \beta is -1 so we
    // can modify this formula:
    //
    // c0' = (c0 + c1) * (c0 - c1)
    // c1' = 2 * c0 * c1

    Fp a = this->c0.add(this->c1);
    Fp b = this->c0.sub(this->c1);
    Fp c = this->c0.add(this->c0);

    return Fp2(a.mul(b),
               c.mul(this->c1));
}

Fp2 Fp2::operator-() const
{
    return this->neg();
}

Fp2 Fp2::operator + (const Fp2& rhs) const
{
    return this->add(rhs);
}

Fp2 Fp2::operator - (const Fp2& rhs) const
{
    return this->sub(rhs);
}

Fp2 Fp2::operator * (const Fp2& rhs) const
{
    return this->mul(rhs);
}
