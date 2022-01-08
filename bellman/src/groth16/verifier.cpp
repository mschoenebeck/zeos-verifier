
#include "groth16/verifier.hpp"

PreparedVerifyingKey prepare_verifying_key(const VerifyingKey& vk)
{
    G2Affine gamma = vk.gamma_g2.neg();
    G2Affine delta = vk.delta_g2.neg();

    PreparedVerifyingKey pvk;
    pvk.alpha_g1_beta_g2 = Pairing::pairing(vk.alpha_g1, vk.beta_g2);
    pvk.neg_gamma_g2 = gamma.into();
    pvk.neg_delta_g2 = delta.into();
    pvk.ic = vk.ic;

    return pvk;
}

bool verify_proof(const PreparedVerifyingKey& pvk,
                  const Proof& proof,
                  const vector<Scalar>& public_inputs)
{
    if((public_inputs.size() + 1) != pvk.ic.size())
    {
        // Verification Error: Invalid VerifyingKey
        return false;
    }

    G1Projective acc = pvk.ic[0].to_curve();

    for(int i = 0; i < public_inputs.size(); i++)
    {
        auto b = pvk.ic[i+1].to_curve();
        auto j = public_inputs[i];
        auto tmp2 = b*j;
        acc = acc + tmp2;
    }

    // The original verification equation is:
    // A * B = alpha * beta + inputs * gamma + C * delta
    // ... however, we rearrange it so that it is:
    // A * B - inputs * gamma - C * delta = alpha * beta
    // or equivalently:
    // A * B + inputs * (-gamma) + C * (-delta) = alpha * beta
    // which allows us to do a single final exponentiation.

    return  pvk.alpha_g1_beta_g2 == Pairing::multi_miller_loop(vector<tuple<G1Affine, G2Prepared>>{
        tuple<G1Affine, G2Prepared>{proof.a, proof.b.into()},
        tuple<G1Affine, G2Prepared>{acc.to_affine(), pvk.neg_gamma_g2},
        tuple<G1Affine, G2Prepared>{proof.c, pvk.neg_delta_g2}}).final_exponentiation();
}
