use rustzeos::{halo2, groth16};
use wasm_bindgen::prelude::wasm_bindgen;

#[wasm_bindgen]
#[allow(non_snake_case)]
#[no_mangle]
pub fn verify_halo2_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    //log("Hello from Rust!");
    //log_u32(vk.to_vec().len() as u32);
    //log_many("Logging", "many values!");

    let proof = halo2::Proof::new(proof.to_vec());
    let inputs = halo2::deserialize_instances(inputs);
    let vk = halo2::VerifyingKey::deserialize(&vk.to_vec());

    halo2::verify_proof(&vk, &proof, &inputs)
}

#[wasm_bindgen]
#[allow(non_snake_case)]
#[no_mangle]
pub fn verify_groth16_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    //log("Hello from Rust!");
    //log_u32(vk.to_vec().len() as u32);
    //log_many("Logging", "many values!");

    let proof = groth16::Proof::read(proof).unwrap();
    let inputs = groth16::deserialize_inputs(inputs);
    let vk = groth16::VerifyingKey::read(vk).unwrap();
    let pvk = groth16::prepare_verifying_key(&vk);

    groth16::verify_proof(&pvk, &proof, &inputs)
}

// from: https://stackoverflow.com/questions/66836479/calling-console-log-from-rust-wasm32-wasi-without-the-need-for-ssvm-ssvmup
#[wasm_bindgen]
extern "C" {
    // Use `js_namespace` here to bind `console.log(..)` instead of just
    // `log(..)`
    #[wasm_bindgen(js_namespace = console)]
    fn log(s: &str);

    // The `console.log` is quite polymorphic, so we can bind it with multiple
    // signatures. Note that we need to use `js_name` to ensure we always call
    // `log` in JS.
    #[wasm_bindgen(js_namespace = console, js_name = log)]
    fn log_u32(a: u32);

    // Multiple arguments too!
    #[wasm_bindgen(js_namespace = console, js_name = log)]
    fn log_many(a: &str, b: &str);
}

#[cfg(test)]
mod tests
{
    /// NOTE:
    /// Because of different versions of dependency 'ff' used in halo2 and groth16 the code
    /// for the other proving system needs to be commented out in order for the tests module
    /// to compile. TODO: solve this by having both proving systems use same ff dependency.
    /// Could be solved by updating bellman crate to version 0.13.x
    
    /* comment this module out when testing groth16 */
    mod halo2
    {
        use core::iter;

        use ff::Field;
        use halo2_proofs::circuit::Value;
        use pasta_curves::pallas;
        use rand::{rngs::OsRng, RngCore};

        use zeos_orchard::circuit::{Circuit, Instance, K};
        use rustzeos::halo2::{Proof, ProvingKey, VerifyingKey};
        use zeos_orchard::{
            keys::SpendValidatingKey,
            note::Note,
            tree::MerklePath,
            value::{NoteValue},
        };
        use rustzeos::halo2::{Instance as ConcreteInstance, serialize_instances};
        use crate::verify_halo2_proof;

        fn generate_circuit_instance<R: RngCore>(mut rng: R) -> (Circuit, Instance) {
            let (_, fvk, note_a) = Note::dummy(&mut rng, None, Some(NoteValue::from_raw(7)));

            let sender_address = note_a.recipient();
            let nk = *fvk.nk();
            let rivk = fvk.rivk(fvk.scope_for_address(&note_a.recipient()).unwrap());
            let nf_a = note_a.nullifier(&fvk);
            let ak: SpendValidatingKey = fvk.into();
            let alpha = pallas::Scalar::random(&mut rng);
            let rk = ak.randomize(&alpha);

            let note_b = Note::new(sender_address, NoteValue::from_raw(3), NoteValue::zero(), NoteValue::zero(), NoteValue::zero(), nf_a, &mut rng);
            let note_c = Note::new(sender_address, NoteValue::from_raw(4), NoteValue::zero(), NoteValue::zero(), NoteValue::zero(), nf_a, &mut rng);

            let path = MerklePath::dummy(&mut rng);
            let anchor = path.root(note_a.commitment().into());

            (
                Circuit {
                    path: Value::known(path.auth_path()),
                    pos: Value::known(path.position()),
                    g_d_a: Value::known(sender_address.g_d()),
                    pk_d_a: Value::known(*sender_address.pk_d()),
                    d1_a: Value::known(note_a.d1()),
                    d2_a: Value::known(note_a.d2()),
                    rho_a: Value::known(note_a.rho()),
                    psi_a: Value::known(note_a.rseed().psi(&note_a.rho())),
                    rcm_a: Value::known(note_a.rseed().rcm(&note_a.rho())),
                    cm_a: Value::known(note_a.commitment()),
                    alpha: Value::known(alpha),
                    ak: Value::known(ak),
                    nk: Value::known(nk),
                    rivk: Value::known(rivk),
                    g_d_b: Value::known(note_b.recipient().g_d()),
                    pk_d_b: Value::known(*note_b.recipient().pk_d()),
                    d1_b: Value::known(note_b.d1()),
                    d2_b: Value::known(note_b.d2()),
                    sc_b: Value::known(note_b.sc()),
                    rho_b: Value::known(nf_a),
                    psi_b: Value::known(note_b.rseed().psi(&note_b.rho())),
                    rcm_b: Value::known(note_b.rseed().rcm(&note_b.rho())),
                    g_d_c: Value::known(sender_address.g_d()),
                    pk_d_c: Value::known(*sender_address.pk_d()),
                    d1_c: Value::known(note_c.d1()),
                    psi_c: Value::known(note_c.rseed().psi(&note_c.rho())),
                    rcm_c: Value::known(note_c.rseed().rcm(&note_c.rho())),
                },
                Instance {
                    anchor: anchor,
                    nf: nf_a,
                    rk: rk,
                    nft: false,
                    b_d1: NoteValue::from_raw(0),
                    b_d2: NoteValue::from_raw(0),
                    b_sc: NoteValue::from_raw(0),
                    c_d1: NoteValue::from_raw(0),
                    cmb: note_b.commitment().into(),
                    cmc: note_c.commitment().into(),
                },
            )
        }

        #[test]
        // cargo test --package zeos-verifier --lib -- tests::test_verify_halo2_proof --exact --nocapture <
        fn test_verify_halo2_proof()
        {
            let mut rng = OsRng;

            let (circuits, instances): (Vec<_>, Vec<_>) = iter::once(())
                .map(|()| generate_circuit_instance(&mut rng))
                .unzip();

            let vk = VerifyingKey::build(Circuit::default(), K);
            let pk = ProvingKey::build(Circuit::default(), K);
            let proof = Proof::create(&pk, &circuits, &instances, &mut rng).unwrap();

            let mut vk_bytes = Vec::new();
            vk.serialize(&mut vk_bytes);
            let instances: Vec<_> = instances.iter().map(|i| i.to_halo2_instance_vec()).collect();
            let inputs_bytes = serialize_instances(&instances);

            println!("proof_bytes = {:?}", proof.as_ref());
            println!("inputs_bytes = {:?}", inputs_bytes);
            println!("vk_bytes = {:?}", vk_bytes);

            assert!(verify_halo2_proof(proof.as_ref(), &inputs_bytes, &vk_bytes));
        }
    }
    
    /* comment this module out when testing halo2 
    mod groth16
    {
        /// This is the main example of the bellman crate taken from:
        /// https://docs.rs/bellman/0.10.0/bellman/

        use bellman::{
            gadgets::{
                boolean::{AllocatedBit, Boolean},
                multipack,
                sha256::sha256,
            },
            groth16, Circuit, ConstraintSystem, SynthesisError,
        };
        use bls12_381::Bls12;
        use ff::PrimeField;
        use rand::rngs::OsRng;
        use sha2::{Digest, Sha256};
        use rustzeos::groth16::serialize_inputs;
        use crate::verify_groth16_proof;
        
        /// Our own SHA-256d gadget. Input and output are in little-endian bit order.
        fn sha256d<Scalar: PrimeField, CS: ConstraintSystem<Scalar>>(
            mut cs: CS,
            data: &[Boolean],
        ) -> Result<Vec<Boolean>, SynthesisError> {
            // Flip endianness of each input byte
            let input: Vec<_> = data
                .chunks(8)
                .map(|c| c.iter().rev())
                .flatten()
                .cloned()
                .collect();
        
            let mid = sha256(cs.namespace(|| "SHA-256(input)"), &input)?;
            let res = sha256(cs.namespace(|| "SHA-256(mid)"), &mid)?;
        
            // Flip endianness of each output byte
            Ok(res
                .chunks(8)
                .map(|c| c.iter().rev())
                .flatten()
                .cloned()
                .collect())
        }
        
        #[test]
        fn test_verify_groth16_proof()
        {
            struct MyCircuit {
                /// The input to SHA-256d we are proving that we know. Set to `None` when we
                /// are verifying a proof (and do not have the witness data).
                preimage: Option<[u8; 80]>,
            }
            
            impl<Scalar: PrimeField> Circuit<Scalar> for MyCircuit {
                fn synthesize<CS: ConstraintSystem<Scalar>>(self, cs: &mut CS) -> Result<(), SynthesisError> {
                    // Compute the values for the bits of the preimage. If we are verifying a proof,
                    // we still need to create the same constraints, so we return an equivalent-size
                    // Vec of None (indicating that the value of each bit is unknown).
                    let bit_values = if let Some(preimage) = self.preimage {
                        preimage
                            .into_iter()
                            .map(|byte| (0..8).map(move |i| (byte >> i) & 1u8 == 1u8))
                            .flatten()
                            .map(|b| Some(b))
                            .collect()
                    } else {
                        vec![None; 80 * 8]
                    };
                    assert_eq!(bit_values.len(), 80 * 8);
            
                    // Witness the bits of the preimage.
                    let preimage_bits = bit_values
                        .into_iter()
                        .enumerate()
                        // Allocate each bit.
                        .map(|(i, b)| {
                            AllocatedBit::alloc(cs.namespace(|| format!("preimage bit {}", i)), b)
                        })
                        // Convert the AllocatedBits into Booleans (required for the sha256 gadget).
                        .map(|b| b.map(Boolean::from))
                        .collect::<Result<Vec<_>, _>>()?;
            
                    // Compute hash = SHA-256d(preimage).
                    let hash = sha256d(cs.namespace(|| "SHA-256d(preimage)"), &preimage_bits)?;
            
                    // Expose the vector of 32 boolean variables as compact public inputs.
                    multipack::pack_into_inputs(cs.namespace(|| "pack hash"), &hash)
                }
            }
            
            // Create parameters for our circuit. In a production deployment these would
            // be generated securely using a multiparty computation.
            let params = {
                let c = MyCircuit { preimage: None };
                groth16::generate_random_parameters::<Bls12, _, _>(c, &mut OsRng).unwrap()
            };
            
            // Pick a preimage and compute its hash.
            let preimage = [42; 80];
            let hash = Sha256::digest(&Sha256::digest(&preimage));
            
            // Create an instance of our circuit (with the preimage as a witness).
            let c = MyCircuit {
                preimage: Some(preimage),
            };
            
            // Create a Groth16 proof with our parameters.
            let proof = groth16::create_random_proof(c, &params, &mut OsRng).unwrap();
            
            // Pack the hash as inputs for proof verification.
            let hash_bits = multipack::bytes_to_bits_le(&hash);
            let inputs: Vec<bls12_381::Scalar> = multipack::compute_multipacking(&hash_bits);
            
            let mut vk_bytes = Vec::<u8>::new();
            assert!(params.vk.write(&mut vk_bytes).is_ok());
            let mut proof_bytes = Vec::<u8>::new();
            assert!(proof.write(&mut proof_bytes).is_ok());
            let inputs_bytes = serialize_inputs(&inputs);

            println!("proof_bytes = {:?}", proof_bytes);
            println!("inputs_bytes = {:?}", inputs_bytes);
            println!("vk_bytes = {:?}", vk_bytes);

            // Check the proof!
            assert!(verify_groth16_proof(&proof_bytes, &inputs_bytes, &vk_bytes));
        }
    }
    */
}
