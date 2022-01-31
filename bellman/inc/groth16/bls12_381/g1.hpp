#pragma once

#include "fp.hpp"
#include "subtle.hpp"
#include "scalar.hpp"

class G1Projective;
class Scalar;

class G1Affine
{
    public:

        // members
        Fp x;
        Fp y;
        Choice infinity;
        JS_OBJ(x, y, infinity);

        G1Affine();
        G1Affine(const Fp& x, const Fp& y, const Choice& infinity);

        static G1Affine from(const G1Projective& p);

        static G1Affine conditional_select(const G1Affine& a, const G1Affine& b, const Choice& choice);

        static G1Affine generator();

        static G1Affine identity();

        Choice is_identity() const;

        G1Projective to_curve() const;
};

class G1Projective
{
    public:

        // members
        Fp x;
        Fp y;
        Fp z;
        JS_OBJ(x, y, z);

        G1Projective();
        G1Projective(const Fp& x, const Fp& y, const Fp& z);

        G1Affine to_affine() const;

        static G1Projective conditional_select(const G1Projective& a, const G1Projective& b, const Choice& choice);

        static G1Projective identity();

        Choice is_identity() const;

        G1Projective dbl() const;

        G1Projective add(const G1Projective& rhs) const;

        G1Projective mul(const Scalar& rhs) const;

        G1Projective operator + (const G1Projective& rhs) const;

        G1Projective operator * (const Scalar& rhs) const;
};




Fp mul_by_3b(const Fp& a);
