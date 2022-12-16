use hex::FromHexError;
use neon::prelude::*;
use rustzeos::{halo2, groth16, zeos};

pub fn verify_halo2_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    let proof = halo2::Proof::new(proof.to_vec());
    let inputs = halo2::deserialize_instances(inputs);
    let vk = halo2::VerifyingKey::deserialize(&vk.to_vec());

    halo2::verify_proof(&vk, &proof, &inputs)
}

pub fn verify_groth16_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    let proof = groth16::Proof::read(proof).unwrap();
    let inputs = groth16::deserialize_inputs(inputs);
    let vk = groth16::VerifyingKey::read(vk).unwrap();
    let pvk = groth16::prepare_verifying_key(&vk);

    groth16::verify_proof(&pvk, &proof, &inputs)
}

pub fn verify_zeos_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    let proof = halo2::Proof::new(proof.to_vec());
    let inputs = zeos::deserialize_instances(inputs);
    let vk = halo2::VerifyingKey::deserialize(&vk.to_vec());

    halo2::verify_proof(&vk, &proof, &inputs)
}

fn decode_inputs_strings(proof_str: String, inputs_str: String, vk_str: String) -> Result<(Vec<u8>, Vec<u8>, Vec<u8>), FromHexError>
{
    assert!(proof_str.len() % 2 == 0);
    assert!(inputs_str.len() % 2 == 0);
    assert!(vk_str.len() % 2 == 0);
    let mut proof_arr = vec![0; proof_str.len() / 2];
    let mut inputs_arr = vec![0; inputs_str.len() / 2];
    let mut vk_arr = vec![0; vk_str.len() / 2];
    hex::decode_to_slice(proof_str, &mut proof_arr)?;
    hex::decode_to_slice(inputs_str, &mut inputs_arr)?;
    hex::decode_to_slice(vk_str, &mut vk_arr)?;
    Ok((proof_arr, inputs_arr, vk_arr))
}

fn js_verify_groth16_proof(mut cx: FunctionContext) -> JsResult<JsBoolean>
{
    let res = decode_inputs_strings(
        cx.argument::<JsString>(0)?.value(&mut cx),
        cx.argument::<JsString>(1)?.value(&mut cx),
        cx.argument::<JsString>(2)?.value(&mut cx)
    );

    if res.is_err()
    {
        return cx.throw_error(res.err().unwrap().to_string());
    }

    let (proof, inputs, vk) = res.unwrap();
    Ok(cx.boolean(verify_groth16_proof(&proof, &inputs, &vk)))
}

fn js_verify_halo2_proof(mut cx: FunctionContext) -> JsResult<JsBoolean>
{
    let res = decode_inputs_strings(
        cx.argument::<JsString>(0)?.value(&mut cx),
        cx.argument::<JsString>(1)?.value(&mut cx),
        cx.argument::<JsString>(2)?.value(&mut cx)
    );

    if res.is_err()
    {
        return cx.throw_error(res.err().unwrap().to_string());
    }

    let (proof, inputs, vk) = res.unwrap();
    Ok(cx.boolean(verify_halo2_proof(&proof, &inputs, &vk)))
}

fn js_verify_zeos_proof(mut cx: FunctionContext) -> JsResult<JsBoolean>
{
    let res = decode_inputs_strings(
        cx.argument::<JsString>(0)?.value(&mut cx),
        cx.argument::<JsString>(1)?.value(&mut cx),
        cx.argument::<JsString>(2)?.value(&mut cx)
    );

    if res.is_err()
    {
        return cx.throw_error(res.err().unwrap().to_string());
    }

    let (proof, inputs, vk) = res.unwrap();
    Ok(cx.boolean(verify_zeos_proof(&proof, &inputs, &vk)))
}

#[neon::main]
fn main(mut cx: ModuleContext) -> NeonResult<()>
{
    cx.export_function("verify_groth16_proof", js_verify_groth16_proof)?;
    cx.export_function("verify_halo2_proof", js_verify_halo2_proof)?;
    cx.export_function("verify_zeos_proof", js_verify_zeos_proof)?;
    Ok(())
}

#[cfg(test)]
mod tests
{
    mod halo2
    {
        /// This is the main example of the halo2 crate taken from:
        /// https://zcash.github.io/halo2/user/simple-example.html

        use std::marker::PhantomData;
        use halo2_proofs::{
            arithmetic::FieldExt,
            circuit::{AssignedCell, Chip, Layouter, Region, SimpleFloorPlanner, Value},
            plonk::{Advice, Circuit, Column, ConstraintSystem, Error, Fixed, Instance, Selector},
            poly::Rotation,
        };

        use rustzeos::halo2::{Proof, ProvingKey, VerifyingKey, Instance as ConcreteInstance, serialize_instances};
        use crate::verify_halo2_proof;
        use rand::rngs::OsRng;

        // ANCHOR: instructions
        trait NumericInstructions<F: FieldExt>: Chip<F> {
            /// Variable representing a number.
            type Num;

            /// Loads a number into the circuit as a private input.
            fn load_private(&self, layouter: impl Layouter<F>, a: Value<F>) -> Result<Self::Num, Error>;

            /// Loads a number into the circuit as a fixed constant.
            fn load_constant(&self, layouter: impl Layouter<F>, constant: F) -> Result<Self::Num, Error>;

            /// Returns `c = a * b`.
            fn mul(
                &self,
                layouter: impl Layouter<F>,
                a: Self::Num,
                b: Self::Num,
            ) -> Result<Self::Num, Error>;

            /// Exposes a number as a public input to the circuit.
            fn expose_public(
                &self,
                layouter: impl Layouter<F>,
                num: Self::Num,
                row: usize,
            ) -> Result<(), Error>;
        }
        // ANCHOR_END: instructions

        // ANCHOR: chip
        /// The chip that will implement our instructions! Chips store their own
        /// config, as well as type markers if necessary.
        struct FieldChip<F: FieldExt> {
            config: FieldConfig,
            _marker: PhantomData<F>,
        }
        // ANCHOR_END: chip

        // ANCHOR: chip-config
        /// Chip state is stored in a config struct. This is generated by the chip
        /// during configuration, and then stored inside the chip.
        #[derive(Clone, Debug)]
        struct FieldConfig {
            /// For this chip, we will use two advice columns to implement our instructions.
            /// These are also the columns through which we communicate with other parts of
            /// the circuit.
            advice: [Column<Advice>; 2],

            /// This is the public input (instance) column.
            instance: Column<Instance>,

            // We need a selector to enable the multiplication gate, so that we aren't placing
            // any constraints on cells where `NumericInstructions::mul` is not being used.
            // This is important when building larger circuits, where columns are used by
            // multiple sets of instructions.
            s_mul: Selector,
        }

        impl<F: FieldExt> FieldChip<F> {
            fn construct(config: <Self as Chip<F>>::Config) -> Self {
                Self {
                    config,
                    _marker: PhantomData,
                }
            }

            fn configure(
                meta: &mut ConstraintSystem<F>,
                advice: [Column<Advice>; 2],
                instance: Column<Instance>,
                constant: Column<Fixed>,
            ) -> <Self as Chip<F>>::Config {
                meta.enable_equality(instance);
                meta.enable_constant(constant);
                for column in &advice {
                    meta.enable_equality(*column);
                }
                let s_mul = meta.selector();

                // Define our multiplication gate!
                meta.create_gate("mul", |meta| {
                    // To implement multiplication, we need three advice cells and a selector
                    // cell. We arrange them like so:
                    //
                    // | a0  | a1  | s_mul |
                    // |-----|-----|-------|
                    // | lhs | rhs | s_mul |
                    // | out |     |       |
                    //
                    // Gates may refer to any relative offsets we want, but each distinct
                    // offset adds a cost to the proof. The most common offsets are 0 (the
                    // current row), 1 (the next row), and -1 (the previous row), for which
                    // `Rotation` has specific constructors.
                    let lhs = meta.query_advice(advice[0], Rotation::cur());
                    let rhs = meta.query_advice(advice[1], Rotation::cur());
                    let out = meta.query_advice(advice[0], Rotation::next());
                    let s_mul = meta.query_selector(s_mul);

                    // Finally, we return the polynomial expressions that constrain this gate.
                    // For our multiplication gate, we only need a single polynomial constraint.
                    //
                    // The polynomial expressions returned from `create_gate` will be
                    // constrained by the proving system to equal zero. Our expression
                    // has the following properties:
                    // - When s_mul = 0, any value is allowed in lhs, rhs, and out.
                    // - When s_mul != 0, this constrains lhs * rhs = out.
                    vec![s_mul * (lhs * rhs - out)]
                });

                FieldConfig {
                    advice,
                    instance,
                    s_mul,
                }
            }
        }
        // ANCHOR_END: chip-config

        // ANCHOR: chip-impl
        impl<F: FieldExt> Chip<F> for FieldChip<F> {
            type Config = FieldConfig;
            type Loaded = ();

            fn config(&self) -> &Self::Config {
                &self.config
            }

            fn loaded(&self) -> &Self::Loaded {
                &()
            }
        }
        // ANCHOR_END: chip-impl

        // ANCHOR: instructions-impl
        /// A variable representing a number.
        #[derive(Clone)]
        struct Number<F: FieldExt>(AssignedCell<F, F>);

        impl<F: FieldExt> NumericInstructions<F> for FieldChip<F> {
            type Num = Number<F>;

            fn load_private(
                &self,
                mut layouter: impl Layouter<F>,
                value: Value<F>,
            ) -> Result<Self::Num, Error> {
                let config = self.config();

                layouter.assign_region(
                    || "load private",
                    |mut region| {
                        region
                            .assign_advice(|| "private input", config.advice[0], 0, || value)
                            .map(Number)
                    },
                )
            }

            fn load_constant(
                &self,
                mut layouter: impl Layouter<F>,
                constant: F,
            ) -> Result<Self::Num, Error> {
                let config = self.config();

                layouter.assign_region(
                    || "load constant",
                    |mut region| {
                        region
                            .assign_advice_from_constant(|| "constant value", config.advice[0], 0, constant)
                            .map(Number)
                    },
                )
            }

            fn mul(
                &self,
                mut layouter: impl Layouter<F>,
                a: Self::Num,
                b: Self::Num,
            ) -> Result<Self::Num, Error> {
                let config = self.config();

                layouter.assign_region(
                    || "mul",
                    |mut region: Region<'_, F>| {
                        // We only want to use a single multiplication gate in this region,
                        // so we enable it at region offset 0; this means it will constrain
                        // cells at offsets 0 and 1.
                        config.s_mul.enable(&mut region, 0)?;

                        // The inputs we've been given could be located anywhere in the circuit,
                        // but we can only rely on relative offsets inside this region. So we
                        // assign new cells inside the region and constrain them to have the
                        // same values as the inputs.
                        a.0.copy_advice(|| "lhs", &mut region, config.advice[0], 0)?;
                        b.0.copy_advice(|| "rhs", &mut region, config.advice[1], 0)?;

                        // Now we can assign the multiplication result, which is to be assigned
                        // into the output position.
                        let value = a.0.value().copied() * b.0.value();

                        // Finally, we do the assignment to the output, returning a
                        // variable to be used in another part of the circuit.
                        region
                            .assign_advice(|| "lhs * rhs", config.advice[0], 1, || value)
                            .map(Number)
                    },
                )
            }

            fn expose_public(
                &self,
                mut layouter: impl Layouter<F>,
                num: Self::Num,
                row: usize,
            ) -> Result<(), Error> {
                let config = self.config();

                layouter.constrain_instance(num.0.cell(), config.instance, row)
            }
        }
        // ANCHOR_END: instructions-impl

        // ANCHOR: circuit
        /// The full circuit implementation.
        ///
        /// In this struct we store the private input variables. We use `Option<F>` because
        /// they won't have any value during key generation. During proving, if any of these
        /// were `None` we would get an error.
        #[derive(Default)]
        struct MyCircuit<F: FieldExt> {
            constant: F,
            a: Value<F>,
            b: Value<F>,
        }

        impl<F: FieldExt> Circuit<F> for MyCircuit<F> {
            // Since we are using a single chip for everything, we can just reuse its config.
            type Config = FieldConfig;
            type FloorPlanner = SimpleFloorPlanner;

            fn without_witnesses(&self) -> Self {
                Self::default()
            }

            fn configure(meta: &mut ConstraintSystem<F>) -> Self::Config {
                // We create the two advice columns that FieldChip uses for I/O.
                let advice = [meta.advice_column(), meta.advice_column()];

                // We also need an instance column to store public inputs.
                let instance = meta.instance_column();

                // Create a fixed column to load constants.
                let constant = meta.fixed_column();

                FieldChip::configure(meta, advice, instance, constant)
            }

            fn synthesize(
                &self,
                config: Self::Config,
                mut layouter: impl Layouter<F>,
            ) -> Result<(), Error> {
                let field_chip = FieldChip::<F>::construct(config);

                // Load our private values into the circuit.
                let a = field_chip.load_private(layouter.namespace(|| "load a"), self.a)?;
                let b = field_chip.load_private(layouter.namespace(|| "load b"), self.b)?;

                // Load the constant factor into the circuit.
                let constant =
                    field_chip.load_constant(layouter.namespace(|| "load constant"), self.constant)?;

                // We only have access to plain multiplication.
                // We could implement our circuit as:
                //     asq  = a*a
                //     bsq  = b*b
                //     absq = asq*bsq
                //     c    = constant*asq*bsq
                //
                // but it's more efficient to implement it as:
                //     ab   = a*b
                //     absq = ab^2
                //     c    = constant*absq
                let ab = field_chip.mul(layouter.namespace(|| "a * b"), a, b)?;
                let absq = field_chip.mul(layouter.namespace(|| "ab * ab"), ab.clone(), ab)?;
                let c = field_chip.mul(layouter.namespace(|| "constant * absq"), constant, absq)?;

                // Expose the result as a public input to the circuit.
                field_chip.expose_public(layouter.namespace(|| "expose c"), c, 0)
            }
        }
        // ANCHOR_END: circuit

        #[test]
        // cargo test --package zeos-verifier --lib -- tests::test_verify_halo2_proof --exact --nocapture <
        fn test_verify_halo2_proof()
        {
            use halo2_proofs::pasta::Fp;

            // ANCHOR: test-circuit
            // The number of rows in our circuit cannot exceed 2^k. Since our example
            // circuit is very small, we can pick a very small value here.
            let k = 4;

            // Prepare the private and public inputs to the circuit!
            let constant = Fp::from(7);
            let a = Fp::from(2);
            let b = Fp::from(3);
            let c = constant * a.square() * b.square();

            // Instantiate the circuit with the private inputs.
            let circuit = MyCircuit {
                constant,
                a: Value::known(a),
                b: Value::known(b),
            };
            let circuit_vk = MyCircuit {
                constant,
                a: Value::default(),
                b: Value::default(),
            };
            let circuit_pk = MyCircuit {
                constant,
                a: Value::default(),
                b: Value::default(),
            };

            // Arrange the public input. We expose the multiplication result in row 0
            // of the instance column, so we position it there in our public inputs.
            let public_inputs = vec![c];

            let mut rng = OsRng;

            pub struct Instance(Vec<Vec<Fp>>);
            impl ConcreteInstance for Instance{
                fn to_halo2_instance_vec(&self) -> Vec<Vec<Fp>> {
                    self.0.clone()
                }
            }

            // create keys and proof
            let vk = VerifyingKey::build(circuit_vk, k);
            let pk = ProvingKey::build(circuit_pk, k);
            let proof = Proof::create(&pk, &[circuit], &[Instance(vec![public_inputs.clone()])], &mut rng).unwrap();

            // serialize data structs to byte arrays
            let mut vk_bytes = Vec::new();
            vk.serialize(&mut vk_bytes);
            let instances: Vec<_> = Instance(vec![public_inputs]).to_halo2_instance_vec();
            let inputs_bytes = serialize_instances(&vec![instances]);

            // as byte arrays
            //println!("proof_bytes = {:02X?}", proof.as_ref());
            //println!("inputs_bytes = {:02X?}", inputs_bytes);
            //println!("vk_bytes = {:02X?}", vk_bytes);
            // as hex strings
            println!("proof_str = {}", hex::encode(proof.as_ref()));
            println!("inputs_str = {}", hex::encode(&inputs_bytes));
            println!("vk_str = {}", hex::encode(&vk_bytes));

            // Check the proof!
            assert!(verify_halo2_proof(proof.as_ref(), &inputs_bytes, &vk_bytes));
        }
    }
    
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
            
            // serialize data structs to byte arrays
            let mut vk_bytes = Vec::<u8>::new();
            assert!(params.vk.write(&mut vk_bytes).is_ok());
            let mut proof_bytes = Vec::<u8>::new();
            assert!(proof.write(&mut proof_bytes).is_ok());
            let inputs_bytes = serialize_inputs(&inputs);

            // as byte arrays
            //println!("proof_bytes = {:?}", proof_bytes);
            //println!("inputs_bytes = {:?}", inputs_bytes);
            //println!("vk_bytes = {:?}", vk_bytes);
            // as hex strings
            println!("proof_str = {}", hex::encode(&proof_bytes));
            println!("inputs_str = {}", hex::encode(&inputs_bytes));
            println!("vk_str = {}", hex::encode(&vk_bytes));

            // Check the proof!
            assert!(verify_groth16_proof(&proof_bytes, &inputs_bytes, &vk_bytes));
        }
    }
}
