
#include "groth16/bls12_381/g2.hpp"

// G2Affine implementations

G2Affine::G2Affine() : x(Fp2()), y(Fp2()), infinity(Choice())
{
}

G2Affine::G2Affine(const Fp2& x, const Fp2& y, const Choice& infinity) : x(x), y(y), infinity(infinity)
{
}

G2Affine G2Affine::conditional_select(const G2Affine& a, const G2Affine& b, const Choice& choice)
{
    return G2Affine(Fp2::conditional_select(a.x, b.x, choice),
                    Fp2::conditional_select(a.y, b.y, choice),
                    Choice::conditional_select(a.infinity, b.infinity, choice));
}

G2Affine G2Affine::generator()
{
    return G2Affine(
                Fp2(Fp::from_raw_unchecked({
                        0xf5f2'8fa2'0294'0a10,
                        0xb3f5'fb26'87b4'961a,
                        0xa1a8'93b5'3e2a'e580,
                        0x9894'999d'1a3c'aee9,
                        0x6f67'b763'1863'366b,
                        0x0581'9192'4350'bcd7,
                    }),
                    Fp::from_raw_unchecked({
                        0xa5a9'c075'9e23'f606,
                        0xaaa0'c59d'bccd'60c3,
                        0x3bb1'7e18'e286'7806,
                        0x1b1a'b6cc'8541'b367,
                        0xc2b6'ed0e'f215'8547,
                        0x1192'2a09'7360'edf3,
                    })),
                Fp2(Fp::from_raw_unchecked({
                        0x4c73'0af8'6049'4c4a,
                        0x597c'fa1f'5e36'9c5a,
                        0xe7e6'856c'aa0a'635a,
                        0xbbef'b5e9'6e0d'495f,
                        0x07d3'a975'f0ef'25a2,
                        0x0083'fd8e'7e80'dae5,
                    }),
                    Fp::from_raw_unchecked({
                        0xadc0'fc92'df64'b05d,
                        0x18aa'270a'2b14'61dc,
                        0x86ad'ac6a'3be4'eba0,
                        0x7949'5c4e'c93d'a33a,
                        0xe717'5850'a43c'caed,
                        0x0b2b'c2a1'63de'1bf2,
                    })),
                Choice(0));
}

Choice G2Affine::is_identity() const
{
    return this->infinity;
}

G2Affine G2Affine::neg() const
{
    return G2Affine(this->x,
                    Fp2::conditional_select(-this->y, Fp2::one(), this->infinity),
                    this->infinity);
}

G2Affine G2Affine::operator-() const
{
    return this->neg();
}

G2Prepared G2Affine::into() const
{
    return G2Prepared::from(*this);
}

// G2Projective implementations

G2Projective::G2Projective() : x(Fp2()), y(Fp2()), z(Fp2())
{
}

G2Projective::G2Projective(const Fp2& x, const Fp2& y, const Fp2& z) : x(x), y(y), z(z)
{
}

G2Projective G2Projective::from(const G2Affine& p)
{
    return G2Projective(p.x,
                        p.y,
                        Fp2::conditional_select(Fp2::one(), Fp2::zero(), p.infinity));
}
