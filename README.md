# ZEOS SNARK Verifier

This is the [vCPU](https://liquidapps.io/vcpu) handler of the [ZEOS Token Contract](https://github.com/mschoenebeck/thezeostoken) to verify zk-SNARKs.

See also:
- [Bellman](https://docs.rs/bellman/latest/bellman/)
- [Halo2](https://zcash.github.io/halo2/index.html)

## Description
This JavaScript handler ('zeos_verify_proof.js') is the vCPU handler of the [ZEOS Token Contract](https://github.com/mschoenebeck/thezeostoken). It supports zk-SNARK verification for both: The [Groth16](https://electriccoin.co/blog/bellman-zksnarks-in-rust/) proving system as well as [Halo2](https://zcash.github.io/halo2/index.html).

This verifier is built for [EOSIO](https://eos.io/) utilizing [Liquidapps' DAPP Network](https://liquidapps.io/) services.

## Getting Started

To setup the full workspace clone the dependencies [rustzeos](https://github.com/mschoenebeck/rustzeos), [bellman](https://github.com/mschoenebeck/bellman) and [halo2](https://github.com/mschoenebeck/halo2) as well:

```
mkdir zeos
cd zeos
git clone https://github.com/mschoenebeck/rustzeos.git
git clone https://github.com/mschoenebeck/halo2.git
git clone https://github.com/mschoenebeck/bellman.git
```

Clone this repository:

```
git clone https://github.com/mschoenebeck/zeos-verifier.git
cd zeos-verifier
```

Build the project as Rust library:

```
cargo build
```

Run the unit tests:

```
cargo test --package zeos-verifier --lib -- tests::groth16::test_verify_groth16_proof --exact --nocapture
cargo test --package zeos-verifier --lib -- tests::halo2::test_verify_halo2_proof --exact --nocapture
```

Build the project as NodeJS extension module:

```
make
```

Run the unit tests:

```
make run
```

### Dependencies

- [Rust Toolchain](https://www.rust-lang.org/tools/install)
- [Node.js](https://nodejs.org/en/)

## Help
If you need help join us on [Telegram](https://t.me/ZeosOnEos).

## Authors

Matthias Schönebeck

## License

It's open source. Do with it whatever you want.

## Acknowledgments

Big thanks to the Electric Coin Company for developing, documenting and maintaining this awesome open source codebase for zk-SNARKs!

* [Zcash Protocol Specification](https://zips.z.cash/protocol/protocol.pdf)
