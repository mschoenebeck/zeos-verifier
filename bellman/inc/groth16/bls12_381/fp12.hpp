#pragma once

#include "fp6.hpp"

/// This represents an element $c_0 + c_1 w$ of $\mathbb{F}_{p^12} = \mathbb{F}_{p^6} / w^2 - v$.
class Fp12
{
    public:

        // members
        Fp6 c0;
        Fp6 c1;
        JS_OBJ(c0, c1);

        Fp12();
        Fp12(const Fp6& c0, const Fp6& c1);

        static Fp12 from(const Fp& f);
        static Fp12 from(const Fp2& f);
        static Fp12 from(const Fp6& f);

        Choice ct_eq(const Fp12& other) const;

        static Fp12 conditional_select(const Fp12& a, const Fp12& b, const Choice& choice);

        static Fp12 zero();

        static Fp12 one();

        Fp12 mul_by_014(const Fp2& c0, const Fp2& c1, const Fp2& c4) const;

        Fp12 conjugate() const;

        Fp12 frobenius_map() const;

        tuple<Fp12, Choice> invert() const;

        Fp12 square() const;

        Fp12 mul(const Fp12& rhs) const;

        Fp12 operator * (const Fp12& rhs) const;
};
