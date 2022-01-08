#pragma once

#include "subtle.hpp"
#include "fp2.hpp"
#include "fp12.hpp"
#include "g1.hpp"
#include "g2.hpp"

#include <vector>
#include <tuple>

using namespace std;

#define BLS_X (0xd201'0000'0001'0000)
#define BLS_X_IS_NEGATIVE (true)

class Gt;
class G2Affine;
class G2Projective;

class MillerLoopResult
{
    public:

        // members
        Fp12 data;

        MillerLoopResult();
        MillerLoopResult(const Fp12& data);

        static tuple<Fp2, Fp2> fp4_square(const Fp2& a, const Fp2& b);

        // Adaptation of Algorithm 5.5.4, Guide to Pairing-Based Cryptography
        // Faster Squaring in the Cyclotomic Subgroup of Sixth Degree Extensions
        // https://eprint.iacr.org/2009/565.pdf
        static Fp12 cyclotomic_square(const Fp12& f);

        static Fp12 cycolotomic_exp(const Fp12& f);

        /// This performs a "final exponentiation" routine to convert the result
        /// of a Miller loop into an element of `Gt` with help of efficient squaring
        /// operation in the so-called `cyclotomic subgroup` of `Fq6` so that
        /// it can be compared with other elements of `Gt`.
        Gt final_exponentiation() const;
};

class Gt
{
    public:

        // members
        Fp12 data;
        JS_OBJ(data);

        Gt();
        Gt(const Fp12& data);

        Choice ct_eq(const Gt& other) const;

        bool eq(const Gt& rhs) const;

        bool operator == (const Gt& rhs) const;
};

namespace JS {
template <typename T>
struct TypeHandler<std::tuple<T, T, T>>
{
public:
  static inline Error to(std::tuple<T, T, T> &to_type, ParseContext &context)
  {
    if (context.token.value_type != JS::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != JS::Error::NoError)
      return error;
    
    to_type = std::tuple<T, T, T>{T(), T(), T()};
    error = TypeHandler<T>::to(get<0>(to_type), context);
    if (error != JS::Error::NoError)
    return error;
    error = context.nextToken();
    if (error != JS::Error::NoError)
    return error;

    error = TypeHandler<T>::to(get<1>(to_type), context);
    if (error != JS::Error::NoError)
    return error;
    error = context.nextToken();
    if (error != JS::Error::NoError)
    return error;

    error = TypeHandler<T>::to(get<2>(to_type), context);
    if (error != JS::Error::NoError)
    return error;
    error = context.nextToken();
    if (error != JS::Error::NoError)
    return error;

    if (context.token.value_type != JS::Type::ArrayEnd)
      return Error::ExpectedArrayEnd;
    
    return Error::NoError;
  }

  static inline void from(const std::tuple<T, T, T> &tup, Token &token, Serializer &serializer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    serializer.write(token);

    token.name = DataRef("");

    T a, b, c;
    tie(a, b, c) = tup;
    TypeHandler<T>::from(a, token, serializer);
    TypeHandler<T>::from(b, token, serializer);
    TypeHandler<T>::from(c, token, serializer);

    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    serializer.write(token);
  }
};
}

class G2Prepared
{
    public:

        // members
        Choice infinity;
        vector<tuple<Fp2, Fp2, Fp2>> coeffs;
        JS_OBJ(infinity, coeffs);

        G2Prepared();
        G2Prepared(const Choice& infinity, const vector<tuple<Fp2, Fp2, Fp2>>& coeffs);

        static G2Prepared from(const G2Affine& q);
};

template<typename Output> class MillerLoopDriver
{
    public:
        virtual Output doubling_step(Output& f) = 0;
        virtual Output addition_step(Output& f) = 0;
        virtual Output square_output(const Output& f) = 0;
        virtual Output conjugate(const Output& f) = 0;
        virtual Output one() = 0;
};

class Pairing
{
    public:

        template<typename O> static O miller_loop(MillerLoopDriver<O>& driver);

        static Gt pairing(const G1Affine& p, const G2Affine& q);

        static MillerLoopResult multi_miller_loop(const vector<tuple<G1Affine, G2Prepared>>& terms);

        static Fp12 ell(const Fp12& f, const tuple<Fp2, Fp2, Fp2>& coeffs, const G1Affine& p);

        static tuple<Fp2, Fp2, Fp2> doubling_step(G2Projective& r);

        static tuple<Fp2, Fp2, Fp2> addition_step(G2Projective& r, const G2Affine& q);
};
