#pragma once

#include "fp.hpp"

class Fp2
{
    public:

        // members
        Fp c0;
        Fp c1;
        JS_OBJ(c0, c1);

        Fp2();
        Fp2(const Fp& c0, const Fp& c1);

        static Fp2 from(const Fp& f);

        Choice ct_eq(const Fp2& other) const;

        static Fp2 conditional_select(const Fp2& a, const Fp2& b, const Choice& choice);

        Fp2 frobenius_map() const;

        Fp2 conjugate() const;

        Fp2 mul_by_nonresidue() const;

        static Fp2 zero();

        static Fp2 one();

        tuple<Fp2, Choice> invert() const;

        Fp2 add(const Fp2& rhs) const;

        Fp2 neg() const;

        Fp2 sub(const Fp2& rhs) const;

        Fp2 mul(const Fp2& rhs) const;

        Fp2 square() const;

        Fp2 operator-() const;

        Fp2 operator + (const Fp2& rhs) const;

        Fp2 operator - (const Fp2& rhs) const;

        Fp2 operator * (const Fp2& rhs) const;
};
