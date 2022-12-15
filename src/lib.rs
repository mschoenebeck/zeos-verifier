use hex::FromHexError;
use neon::prelude::*;
use rustzeos::{halo2, groth16, zeos};

fn verify_halo2_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    let proof = halo2::Proof::new(proof.to_vec());
    let inputs = halo2::deserialize_instances(inputs);
    let vk = halo2::VerifyingKey::deserialize(&vk.to_vec());

    halo2::verify_proof(&vk, &proof, &inputs)
}

fn verify_groth16_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
{
    let proof = groth16::Proof::read(proof).unwrap();
    let inputs = groth16::deserialize_inputs(inputs);
    let vk = groth16::VerifyingKey::read(vk).unwrap();
    let pvk = groth16::prepare_verifying_key(&vk);

    groth16::verify_proof(&pvk, &proof, &inputs)
}

fn verify_zeos_proof(proof: &[u8], inputs: &[u8], vk: &[u8]) -> bool
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
