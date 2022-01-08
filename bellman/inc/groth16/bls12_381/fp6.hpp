#pragma once

#include "fp2.hpp"

/// This represents an element $c_0 + c_1 v + c_2 v^2$ of $\mathbb{F}_{p^6} = \mathbb{F}_{p^2} / v^3 - u - 1$.
class Fp6
{
    public:

        // members
        Fp2 c0;
        Fp2 c1;
        Fp2 c2;
        JS_OBJ(c0, c1, c2);

        Fp6();
        Fp6(const Fp2& c0, const Fp2& c1, const Fp2& c2);

        static Fp6 from(const Fp& f);
        static Fp6 from(const Fp2& f);

        Choice ct_eq(const Fp6& other) const;

        static Fp6 conditional_select(const Fp6& a, const Fp6& b, const Choice& choice);

        static Fp6 zero();

        static Fp6 one();

        Fp6 frobenius_map() const;

        Fp6 mul_by_1(const Fp2& c1) const;

        Fp6 mul_by_01(const Fp2& c0,const Fp2& c1) const;

        Fp6 mul_by_nonresidue() const;

        tuple<Fp6, Choice> invert() const;

        Fp6 square() const;

        Fp6 add(const Fp6& rhs) const;

        Fp6 neg() const;

        Fp6 sub(const Fp6& rhs) const;

        Fp6 mul(const Fp6& rhs) const;

        Fp6 operator-() const;

        Fp6 operator + (const Fp6& rhs) const;

        Fp6 operator - (const Fp6& rhs) const;

        Fp6 operator * (const Fp6& rhs) const;
};
