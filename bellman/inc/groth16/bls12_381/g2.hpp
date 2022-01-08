#pragma once

#include "fp2.hpp"
#include "subtle.hpp"
#include "pairing.hpp"

class G2Prepared;

class G2Affine
{
    public:

        // members
        Fp2 x;
        Fp2 y;
        Choice infinity;
        JS_OBJ(x, y, infinity);

        G2Affine();
        G2Affine(const Fp2& x, const Fp2& y, const Choice& infinity);

        static G2Affine conditional_select(const G2Affine& a, const G2Affine& b, const Choice& choice);

        static G2Affine generator();

        Choice is_identity() const;

        G2Affine neg() const;

        G2Affine operator-() const;

        G2Prepared into() const;
};

class G2Projective
{
    public:

        // members
        Fp2 x;
        Fp2 y;
        Fp2 z;
        JS_OBJ(x, y, z);

        G2Projective();
        G2Projective(const Fp2& x, const Fp2& y, const Fp2& z);

        static G2Projective from(const G2Affine& p);
};
