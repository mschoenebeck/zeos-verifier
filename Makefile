all: zeos.js

# Make sure to source emsdk_env.sh before build:
# source ~/eclipse-workspace-cpp/zeos/emsdk/emsdk_env.sh
zeos.js zeos.wasm: zeos.cpp
	em++ -I json_struct/include -I cpp-base64 -I zeos-bellman/inc -O3 -s WASM=1 -s MODULARIZE=1 -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "allocate", "intArrayFromString", "ALLOC_NORMAL"]' -o zeos.js zeos.cpp zeos-bellman/src/groth16/verifier.cpp zeos-bellman/src/groth16/bls12_381/*.cpp cpp-base64/base64.cpp

run: test.js zeos.js zeos.wasm
#	node --trace-warnings test.js
	node test.js

clean:
	rm -rf zeos.js zeos.wasm
