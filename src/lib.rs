use rustzeos::halo2::{Proof, VerifyingKey, deserialize_instances};
use wasm_bindgen::prelude::wasm_bindgen;

#[wasm_bindgen]
#[allow(non_snake_case)]
#[no_mangle]
pub fn verify_halo2_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    //log("Hello from Rust!");
    //log_u32(vk.to_vec().len() as u32);
    //log_many("Logging", "many values!");

    let proof = Proof::new(proof.to_vec());
    let inputs = deserialize_instances(inputs);
    let vk = VerifyingKey::deserialize(&vk.to_vec());

    proof.verify(&vk, &inputs).is_ok()
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
mod tests {
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

        // serialize vk
        let mut vk_arr = Vec::new();
        vk.serialize(&mut vk_arr);

        let instances: Vec<_> = instances.iter().map(|i| i.to_halo2_instance_vec()).collect();
        let inputs = serialize_instances(&instances);

        //println!("proof = {:?}", proof.as_ref());
        //println!("inputs = {:?}", inputs);
        //println!("vk = {:?}", vk_arr);

        assert_eq!(true, verify_halo2_proof(proof.as_ref(), &inputs, &vk_arr));
    }
}
