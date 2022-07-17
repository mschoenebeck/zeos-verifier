#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <json_struct.h>
#include "groth16/verifier.hpp"

#include <emscripten.h>

extern "C" {

EMSCRIPTEN_KEEPALIVE
bool verify_proof(char* vk_str, char* proof_str, char* inputs_str)
{
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
