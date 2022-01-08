#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "json_struct.h"
#include "base64.h"
#include "groth16/verifier.hpp"

#include <emscripten.h>

extern "C" {

EMSCRIPTEN_KEEPALIVE
//bool verify_proof(char* vk_b46, char* proof_b46, char* inputs_b46)
bool verify_proof(char* vk_str, char* proof_str, char* inputs_str)
{
    //std::string vk_str = base64_decode(vk_b46);
    //std::string proof_str = base64_decode(proof_b46);
    //std::string inputs_str = base64_decode(inputs_b46);
    
    Proof proof;
    VerifyingKey vk;
    PreparedVerifyingKey pvk;
    vector<Scalar> inputs;

    JS::ParseContext parseContext(vk_str);
    parseContext.parseTo(vk);
    
    parseContext = JS::ParseContext(proof_str);
    parseContext.parseTo(proof);
    
    parseContext = JS::ParseContext(inputs_str);
    parseContext.parseTo(inputs);
    
    pvk = prepare_verifying_key(vk);
    
    return verify_proof(pvk, proof, inputs);
}

EMSCRIPTEN_KEEPALIVE
int add(int a, int b)
{
    return a + b;
}

EMSCRIPTEN_KEEPALIVE
void sayHi()
{
    printf("Hi!\n");
}

EMSCRIPTEN_KEEPALIVE
int daysInWeek()
{
    return 7;
}

}
