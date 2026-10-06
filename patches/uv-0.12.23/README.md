# uv 0.12.23 z/OS cross-build patch set

This directory records the complete source changes used for the published
`s390x-ibm-zos` `uv` and `uvx` executables in release `v0.12.23`.

## Contents

- `uv-0.12.23-zos.patch` — changes to the upstream uv 0.12.23 workspace.
- `Cargo.lock.zos` — exact resolved lockfile used by the successful build.
- `dependencies/*.patch` — patches for every modified registry dependency.
- `rustc-procmacro-wrapper.sh` — injects Cargo's exact PowerPC host proc macros.
- `zos_uv_stubs.c` — compatibility symbols linked through
  `libzos_mlock_stubs.a`.

The dependency patches are version-specific. Apply each patch to the matching
crate version shown in its filename, then add the resulting directories under
`[patch.crates-io]` in the uv workspace. The successful build used these
important aligned versions:

- libc 0.2.189
- mio 1.2.2
- socket2 0.6.5
- rustix 1.1.4
- tokio 1.53.1
- ring 0.17.14
- rustls 0.23.45 with the ring provider
- nix 0.31.3
- getrandom 0.2.17 and 0.4.3

Do not regenerate the lockfile. Copy `Cargo.lock.zos` to `Cargo.lock` after the
path overrides have been configured.

## Build

The IBM Rust 1.86 toolchain is older than uv's declared MSRV. This build
intentionally uses `--ignore-rust-version`, `RUSTC_BOOTSTRAP=1`, and targeted
feature gates instead of upgrading dependency versions.

```sh
export RUSTC_BOOTSTRAP=1
export CARGO_TARGET_S390X_IBM_ZOS_LINKER=/path/to/s390x-ibm-zos-cc
export CARGO_TARGET_S390X_IBM_ZOS_AR=/path/to/s390x-ibm-zos-ar
export RUSTC_WRAPPER=/path/to/rustc-procmacro-wrapper.sh
export RUSTFLAGS='-C target-feature=-vector -C link-arg=/path/to/libzos_mlock_stubs.a'

cargo build --profile zos --target s390x-ibm-zos \
  --bin uv --bin uvx --ignore-rust-version --jobs 1
```

Both binaries are required. `uvx` is a separate upstream executable and must
remain beside `uv`; it must not be replaced by a symlink.

Published artifact checksums:

- `uv`: `4604c0bbc3c9b549394c9e5b894f18a94f87ff08138813313bd4023858c41616`
- `uvx`: `12a4ceab44ad47975cea9ef360b7c75a904365ab3528e4bf8b361bc65d14b564`

Validated directly on z/OS: `uv --version`, `uvx --help`, `uv python list`,
`uv init`, `uv venv`, TLS installation of `idna` from PyPI, and importing the
installed package.

## Runtime fixes specific to 0.12.23

- Mio uses `pipe` plus `fcntl` on z/OS. Omitting z/OS from this fallback leaves
  descriptors at `-1` and causes `std/src/os/fd/raw.rs: fd != -1` at startup.
- IBM Python emits interpreter-query JSON in IBM-1047 through subprocess pipes.
  The uv Python probe converts that protocol output to UTF-8 before JSON parsing.
- `setdomainname` is an unsupported advisory operation and is supplied by
  `zos_uv_stubs.c` as an `ENOSYS` compatibility shim.
- AWS-LC is replaced by the patched ring TLS provider.
