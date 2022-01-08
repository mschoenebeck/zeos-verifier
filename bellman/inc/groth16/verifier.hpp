#pragma once

#include "bls12_381/g1.hpp"
#include "bls12_381/g2.hpp"
#include "bls12_381/pairing.hpp"

#include <vector>

using namespace std;

// Proof for BLS12-381 curve
typedef struct Proof
{
    G1Affine a;
    G2Affine b;
    G1Affine c;

    JS_OBJ(a, b, c);
} Proof;

typedef struct VerifyingKey
{
    // alpha in g1 for verifying and for creating A/C elements of
    // proof. Never the point at infinity.
    G1Affine alpha_g1;

    // beta in g1 and g2 for verifying and for creating B/C elements
    // of proof. Never the point at infinity.
    G1Affine beta_g1;
    G2Affine beta_g2;

    // gamma in g2 for verifying. Never the point at infinity.
    G2Affine gamma_g2;

    // delta in g1/g2 for verifying and proving, essentially the magic
    // trapdoor that forces the prover to evaluate the C element of the
    // proof with only components from the CRS. Never the point at
    // infinity.
    G1Affine delta_g1;
    G2Affine delta_g2;

    // Elements of the form (beta * u_i(tau) + alpha v_i(tau) + w_i(tau)) / gamma
    // for all public inputs. Because all public inputs have a dummy constraint,
    // this is the same size as the number of inputs, and never contains points
    // at infinity.
    vector<G1Affine> ic;

    JS_OBJ(alpha_g1, beta_g1, beta_g2, gamma_g2, delta_g1, delta_g2, ic);
} VerifyingKey;

typedef struct PreparedVerifyingKey
{
    /// Pairing result of alpha*beta
    Gt alpha_g1_beta_g2;
    /// -gamma in G2
    G2Prepared neg_gamma_g2;
    /// -delta in G2
    G2Prepared neg_delta_g2;
    /// Copy of IC from `VerifiyingKey`.
    vector<G1Affine> ic;

    JS_OBJ(alpha_g1_beta_g2, neg_gamma_g2, neg_delta_g2, ic);
} PreparedVerifyingKey;

PreparedVerifyingKey prepare_verifying_key(const VerifyingKey& vk);

bool verify_proof(const PreparedVerifyingKey& pvk,
                  const Proof& proof,
                  const vector<Scalar>& public_inputs);
