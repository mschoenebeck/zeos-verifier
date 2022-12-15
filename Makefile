all: index.node

./index.node:
	npm i

clean: ./index.node
	rm -rf ./index.node

run: ./index.node test.js
	node test.js
