
#include "groth16/bls12_381/pairing.hpp"

// MillerLoopResult implementations

MillerLoopResult::MillerLoopResult() : data(Fp12())
{
}

MillerLoopResult::MillerLoopResult(const Fp12& data) : data(data)
{
}

tuple<Fp2, Fp2> MillerLoopResult::fp4_square(const Fp2& a, const Fp2& b)
{
    Fp2 t0 = a.square();
    Fp2 t1 = b.square();
    Fp2 t2 = t1.mul_by_nonresidue();
    Fp2 c0 = t2 + t0;
    t2 = a + b;
    t2 = t2.square();
    t2 = t2 - t0;
    Fp2 c1 = t2 - t1;

    return tuple<Fp2, Fp2>{c0, c1};
}

Fp12 MillerLoopResult::cyclotomic_square(const Fp12& f)
{
    Fp2 z0 = f.c0.c0;
    Fp2 z4 = f.c0.c1;
    Fp2 z3 = f.c0.c2;
    Fp2 z2 = f.c1.c0;
    Fp2 z1 = f.c1.c1;
    Fp2 z5 = f.c1.c2;

    Fp2 t0, t1, t2, t3;
    tie(t0, t1) = fp4_square(z0, z1);

    // For A
    z0 = t0 - z0;
    z0 = z0 + z0 + t0;

    z1 = t1 + z1;
    z1 = z1 + z1 + t1;

    tie(t0, t1) = fp4_square(z2, z3);
    tie(t2, t3) = fp4_square(z4, z5);

    // For C
    z4 = t0 - z4;
    z4 = z4 + z4 + t0;

    z5 = t1 + z5;
    z5 = z5 + z5 + t1;

    // For B
    t0 = t3.mul_by_nonresidue();
    z2 = t0 + z2;
    z2 = z2 + z2 + t0;

    z3 = t2 - z3;
    z3 = z3 + z3 + t2;

    return Fp12(Fp6(z0, z4, z3), Fp6(z2, z1, z5));
}

Fp12 MillerLoopResult::cycolotomic_exp(const Fp12& f)
{
    Fp12 tmp = Fp12::one();
    bool found_one = false;
    for(int b=63; b>=0; b--)
    {
        bool i = ((BLS_X >> b) & 1) == 1;
        if(found_one)
        {
            tmp = cyclotomic_square(tmp);
        }
        else
        {
            found_one = i;
        }

        if(i)
        {
            tmp = tmp * f;
        }
    }

    return tmp.conjugate();
}

Gt MillerLoopResult::final_exponentiation() const
{
    Fp12 f = this->data;
    Fp12 t0 = f.frobenius_map()
               .frobenius_map()
               .frobenius_map()
               .frobenius_map()
               .frobenius_map()
               .frobenius_map();
    Fp12 t1;
    Choice choice;
    tie(t1, choice) = f.invert();
    return Gt(Fp12::conditional_select(Fp12(), [&](){
        Fp12 t2 = t0 * t1;
        t1 = t2;
        t2 = t2.frobenius_map().frobenius_map();
        t2 = t2 * t1;
        t1 = cyclotomic_square(t2).conjugate();
        Fp12 t3 = cycolotomic_exp(t2);
        Fp12 t4 = cyclotomic_square(t3);
        Fp12 t5 = t1 * t3;
        t1 = cycolotomic_exp(t5);
        t0 = cycolotomic_exp(t1);
        Fp12 t6 = cycolotomic_exp(t0);
        t6 = t6 * t4;
        t4 = cycolotomic_exp(t6);
        t5 = t5.conjugate();
        t4 = t4 * (t5 * t2);
        t5 = t2.conjugate();
        t1 = t1 * t2;
        t1 = t1.frobenius_map().frobenius_map().frobenius_map();
        t6 = t6 * t5;
        t6 = t6.frobenius_map();
        t3 = t3 * t0;
        t3 = t3.frobenius_map().frobenius_map();
        t3 = t3 * t1;
        t3 = t3 * t6;
        f = t3 * t4;

        return f;
    }(), choice));
}

// Gt implementations

Gt::Gt() : data(Fp12())
{
}

Gt::Gt(const Fp12& data) : data(data)
{
}

Choice Gt::ct_eq(const Gt& other) const
{
    return this->data.ct_eq(other.data);
}

bool Gt::eq(const Gt& rhs) const
{
    return 1 == this->ct_eq(rhs).data;
}

bool Gt::operator == (const Gt& rhs) const
{
    return this->eq(rhs);
}

// G2Prepared implementations

G2Prepared::G2Prepared() : infinity(Choice()), coeffs(vector<tuple<Fp2, Fp2, Fp2>>())
{
}

G2Prepared::G2Prepared(const Choice& infinity, const vector<tuple<Fp2, Fp2, Fp2>>& coeffs) : infinity(infinity), coeffs(coeffs)
{
}

G2Prepared G2Prepared::from(const G2Affine& q)
{
    class Adder
    {
        public:
            G2Projective cur;
            G2Affine base;
            vector<tuple<Fp2, Fp2, Fp2>> coeffs;

            Adder(const G2Projective& cur, const G2Affine& base, const vector<tuple<Fp2, Fp2, Fp2>>& coeffs) : cur(cur), base(base), coeffs(coeffs)
            {
            }
            void doubling_step()
            {
                this->coeffs.push_back(Pairing::doubling_step(this->cur));
            }
            void addition_step()
            {
                this->coeffs.push_back(Pairing::addition_step(this->cur, this->base));
            }
    };

    Choice is_identity = q.is_identity();
    G2Affine qq = G2Affine::conditional_select(q, G2Affine::generator(), is_identity);

    Adder adder(G2Projective::from(qq), qq, vector<tuple<Fp2, Fp2, Fp2>>());

    bool found_one = false;
    for(int b=63; b>=0; b--)
    {
        bool i = (((BLS_X >>1) >> b) & 1) == 1;
        if(!found_one)
        {
            found_one = i;
            continue;
        }

        adder.doubling_step();

        if(i)
        {
            adder.addition_step();
        }
    }
    adder.doubling_step();

    //assert_eq!(adder.coeffs.len(), 68);

    return G2Prepared(is_identity, adder.coeffs);
}

// pairing implementations

template<typename O> O Pairing::miller_loop(MillerLoopDriver<O>& driver)
{
    auto f = driver.one();

    bool found_one = false;
    for(int b=63; b>=0; b--)
    {
        bool i = (((BLS_X >>1) >> b) & 1) == 1;
        if(!found_one)
        {
            found_one = i;
            continue;
        }

        f = driver.doubling_step(f);

        if(i)
        {
            f = driver.addition_step(f);
        }

        f = driver.square_output(f);
    }

    f = driver.doubling_step(f);

    if(BLS_X_IS_NEGATIVE)
    {
        f = driver.conjugate(f);
    }

    return f;
}

Gt Pairing::pairing(const G1Affine& p, const G2Affine& q)
{
    class Adder : public MillerLoopDriver<Fp12>
    {
        public:
            G2Projective cur;
            G2Affine base;
            G1Affine p;

            Adder(const G2Projective& cur, const G2Affine& base, const G1Affine& p) : cur(cur), base(base), p(p)
            {
            }
            Fp12 doubling_step(Fp12& f)
            {
                tuple<Fp2, Fp2, Fp2> coeffs = Pairing::doubling_step(this->cur);
                return Pairing::ell(f, coeffs, this->p);
            }
            Fp12 addition_step(Fp12& f)
            {
                tuple<Fp2, Fp2, Fp2> coeffs = Pairing::addition_step(this->cur, this->base);
                return Pairing::ell(f, coeffs, this->p);
            }
            Fp12 square_output(const Fp12& f)
            {
                return f.square();
            }
            Fp12 conjugate(const Fp12& f)
            {
                return f.conjugate();
            }
            Fp12 one()
            {
                return Fp12::one();
            }
    };

    Choice either_identity = p.is_identity() | q.is_identity();
    G1Affine pp = G1Affine::conditional_select(p, G1Affine::generator(), either_identity);
    G2Affine qq = G2Affine::conditional_select(q, G2Affine::generator(), either_identity);

    Adder adder(G2Projective::from(qq), qq, pp);

    Fp12 tmp = Pairing::miller_loop<Fp12>(adder);
    MillerLoopResult res = MillerLoopResult(Fp12::conditional_select(tmp, Fp12::one(), either_identity));
    return res.final_exponentiation();
}

MillerLoopResult Pairing::multi_miller_loop(const vector<tuple<G1Affine, G2Prepared>>& terms)
{
    class Adder : public MillerLoopDriver<Fp12>
    {
        public:
            vector<tuple<G1Affine, G2Prepared>> terms;
            uint64_t index;

            Adder(const vector<tuple<G1Affine, G2Prepared>>& terms, const uint64_t& index) : terms(terms), index(index)
            {
            }
            Fp12 doubling_step(Fp12& f)
            {
                for(auto term : this->terms)
                {
                    Choice either_identity = get<0>(term).is_identity() | get<1>(term).infinity;

                    Fp12 new_f = Pairing::ell(f, get<1>(term).coeffs[this->index], get<0>(term));
                    f = Fp12::conditional_select(new_f, f, either_identity);
                }
                this->index += 1;

                return f;
            }
            Fp12 addition_step(Fp12& f)
            {
                for(auto term : this->terms)
                {
                    Choice either_identity = get<0>(term).is_identity() | get<1>(term).infinity;

                    Fp12 new_f = Pairing::ell(f, get<1>(term).coeffs[this->index], get<0>(term));
                    f = Fp12::conditional_select(new_f, f, either_identity);
                }
                this->index += 1;

                return f;
            }
            Fp12 square_output(const Fp12& f)
            {
                return f.square();
            }
            Fp12 conjugate(const Fp12& f)
            {
                return f.conjugate();
            }
            Fp12 one()
            {
                return Fp12::one();
            }
    };

    Adder adder(terms, 0);
    Fp12 tmp = miller_loop<Fp12>(adder);
    return MillerLoopResult(tmp);
}

Fp12 Pairing::ell(const Fp12& f, const tuple<Fp2, Fp2, Fp2>& coeffs, const G1Affine& p)
{
    Fp2 c0, c1, c2;
    tie(c0, c1, c2) = coeffs;

    c0.c0 = c0.c0 * p.y;
    c0.c1 = c0.c1 * p.y;

    c1.c0 = c1.c0 * p.x;
    c1.c1 = c1.c1 * p.x;

    return f.mul_by_014(c2, c1, c0);
}

tuple<Fp2, Fp2, Fp2> Pairing::doubling_step(G2Projective& r)
{
    // Adaptation of Algorithm 26, https://eprint.iacr.org/2010/354.pdf
    Fp2 tmp0 = r.x.square();
    Fp2 tmp1 = r.y.square();
    Fp2 tmp2 = tmp1.square();
    Fp2 tmp3 = (tmp1 + r.x).square() - tmp0 - tmp2;
        tmp3 = tmp3 + tmp3;
    Fp2 tmp4 = tmp0 + tmp0 + tmp0;
    Fp2 tmp6 = r.x + tmp4;
    Fp2 tmp5 = tmp4.square();
    Fp2 zsquared = r.z.square();
    r.x  = tmp5 - tmp3 - tmp3;
    r.z  = (r.z + r.y).square() - tmp1 - zsquared;
    r.y  = (tmp3 - r.x) * tmp4;
    tmp2 = tmp2 + tmp2;
    tmp2 = tmp2 + tmp2;
    tmp2 = tmp2 + tmp2;
    r.y  = r.y - tmp2;
    tmp3 = tmp4 * zsquared;
    tmp3 = tmp3 + tmp3;
    tmp3 = -tmp3;
    tmp6 = tmp6.square() - tmp0 - tmp5;
    tmp1 = tmp1 + tmp1;
    tmp1 = tmp1 + tmp1;
    tmp6 = tmp6 - tmp1;
    tmp0 = r.z * zsquared;
    tmp0 = tmp0 + tmp0;

    return tuple<Fp2, Fp2, Fp2>{tmp0, tmp3, tmp6};
}

tuple<Fp2, Fp2, Fp2> Pairing::addition_step(G2Projective& r, const G2Affine& q)
{
    // Adaptation of Algorithm 27, https://eprint.iacr.org/2010/354.pdf
    Fp2 zsquared = r.z.square();
    Fp2 ysquared = q.y.square();
    Fp2 t0 = zsquared * q.x;
    Fp2 t1 = ((q.y + r.z).square() - ysquared - zsquared) * zsquared;
    Fp2 t2 = t0 - r.x;
    Fp2 t3 = t2.square();
    Fp2 t4 = t3 + t3;
        t4 = t4 + t4;
    Fp2 t5 = t4 * t2;
    Fp2 t6 = t1 - r.y - r.y;
    Fp2 t9 = t6 * q.x;
    Fp2 t7 = t4 * r.x;
        r.x = t6.square() - t5 - t7 - t7;
        r.z = (r.z + t2).square() - zsquared - t3;
    Fp2 t10 = q.y + r.z;
    Fp2 t8 = (t7 - r.x) * t6;
        t0 = r.y * t5;
        t0 = t0 + t0;
        r.y = t8 - t0;
        t10 = t10.square() - ysquared;
    Fp2 ztsquared = r.z.square();
        t10 = t10 - ztsquared;
        t9 = t9 + t9 - t10;
        t10 = r.z + r.z;
        t6 = -t6;
        t1 = t6 + t6;

    return tuple<Fp2, Fp2, Fp2>{t10, t1, t9};
}
