NAME="zeos_groth16"

all: $NAME.js

# Make sure to source emsdk_env.sh before build:
# source ~/eclipse-workspace-cpp/zeos/emsdk/emsdk_env.sh
$NAME.js $NAME.wasm: $NAME.cpp
	em++ -I ../json_struct/include -I ../zeos-bellman/inc -O3 -s WASM=1 -s MODULARIZE=1 -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "allocate", "intArrayFromString", "ALLOC_NORMAL"]' -o $NAME.js $NAME.cpp ../zeos-bellman/src/groth16/verifier.cpp ../zeos-bellman/src/groth16/bls12_381/*.cpp

run: test.js $NAME.js $NAME.wasm
#	node --trace-warnings test.js
	node test.js

clean:
	rm -rf $NAME.js $NAME.wasm
	
