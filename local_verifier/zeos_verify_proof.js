const zeos = import('../pkg/zeos_verifier.js');

const fetch = require('node-fetch');
global.fetch = fetch;

// TODO: might require: npm i --save @liquidapps/dapp-client.
const { createClient } = require('@liquidapps/dapp-client');

// Convert a hex string to a byte array
function hex2Bytes(hex)
{
  for(var bytes = [], c = 0; c < hex.length; c += 2)
      bytes.push(parseInt(hex.substr(c, 2), 16));
  return bytes;
}

(async () => {
  var address = "zeos/zb2rhi4yjLrntm4h1rVwx8vbwujun9dSfVWnFZW6bd4a3Fg4j/zb2rhn5B6gUsTCmFwWQ5cHwW9V2PAtw8PkQJTTDyaRW3VDnvS/00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000DFCFD41CE762791C925086AB8C545A455EFBBF5847AE9BEA348697208FF8A218FA8CA4AB03397C28BA30F18AB45C39665DCB6570BA99F41BD99C9C743B2C09250010270000000000000400454F5300000000A6823403EA3055000000000000000091ACD9176F478A92C5B1E18BD793F3B0C7AA6D7E4B1C0F708790C27A203EC9050000000000000000000000000000000000000000000000000000000000000000";

  // split address to extract parameters
  const payloadParts = address.split('/');
  let idx = 0;
  var type = payloadParts[idx++]; // 'groth16' or 'halo2'
  var vk_ipfs_uri = payloadParts[idx++];
  var proof_str = payloadParts[idx++];
  var inputs_str = payloadParts[idx++];

  // fetch verifier key from liquid storage
  // TODO: for zeus use port 13015, for live environment on DSPs use port 3115
  const response = await fetch('https://kylin-dsp-1.liquidapps.io/v1/dsp/liquidstorag/get_uri', {
    method: 'POST',
    mode: 'cors',
    body: JSON.stringify({ uri: 'ipfs://' + vk_ipfs_uri })
  });
  const resJson = await response.json();
  var vk_str = Buffer.from(resJson.data, 'base64').toString();
  console.log(vk_str);
  
  // fetch proof from liquid storage if it is not passed inline
  if(proof_str.substr(0, 1) == "z")
  {
    const _response = await fetch('https://kylin-dsp-1.liquidapps.io/v1/dsp/liquidstorag/get_uri', {
      method: 'POST',
      mode: 'cors',
      body: JSON.stringify({ uri: 'ipfs://' + proof_str })
    });
    const _resJson = await _response.json();
    proof_str = Buffer.from(_resJson.data, 'base64').toString();
  }
  console.log(proof_str);

  var res = false;
  await zeos.then(m => {
    switch(type)
    {
      case "halo2":
        res = m.verify_halo2_proof(hex2Bytes(proof_str), hex2Bytes(inputs_str), hex2Bytes(vk_str));
        break;

      case "groth16":
        res = m.verify_groth16_proof(hex2Bytes(proof_str), hex2Bytes(inputs_str), hex2Bytes(vk_str));
        break;

      case "zeos":
        res = m.verify_zeos_proof(hex2Bytes(proof_str), hex2Bytes(inputs_str), hex2Bytes(vk_str));
        break;

      default:
        res = false;
        break;
    }
  }).catch(console.error);

  if(res)
  {
    res = "1";
  }
  else
  {
    res = "0";
  }

  console.log(res);
})();
