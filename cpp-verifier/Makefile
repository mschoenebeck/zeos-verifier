
all: zeos_groth16.js

# Make sure to source emsdk_env.sh before build:
# source ~/eclipse-workspace-cpp/zeos/emsdk/emsdk_env.sh
zeos_groth16.js zeos_groth16.wasm: zeos_groth16.cpp
	em++ -I ../../json_struct/include -I ../../zeos-bellman/inc -O3 -s WASM=1 -s MODULARIZE=1 -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "allocate", "intArrayFromString", "ALLOC_NORMAL"]' -o zeos_groth16.js zeos_groth16.cpp ../../zeos-bellman/src/groth16/verifier.cpp ../../zeos-bellman/src/groth16/bls12_381/*.cpp

run: test.js zeos_groth16.js zeos_groth16.wasm
#	node --trace-warnings test.js
	node test.js

clean:
	rm -rf zeos_groth16.js zeos_groth16.wasm
	
