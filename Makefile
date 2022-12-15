all: index.node

./index.node:
	npm i

clean:
	rm -rf ./index.node ./target

run: ./index.node test.js
	node test.js
