all: ./pkg

./pkg:
	wasm-pack build --target nodejs

clean: ./pkg
	rm -rf ./pkg

run: ./pkg test.js
	node test.js
