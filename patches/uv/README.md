# uv 0.8.13 z/OS Cargo patches

## Cargo.toml — `[patch.crates-io]` additions

Add the following to uv's workspace `Cargo.toml` (merged with the existing
`[patch.crates-io]` block that patches `reqwest-middleware`/`reqwest-retry`):

```toml
[patch.crates-io]
# existing uv patches
reqwest-middleware = { git = "...", rev = "..." }
reqwest-retry = { git = "...", rev = "..." }

# z/OS crate patches (paths relative to where patches/ are applied)
libc = { path = "patches/libc/libc-0.2.186-zos" }
getrandom = { path = "patches/getrandom-02/getrandom-0.2.17-zos" }
"getrandom04" = { package = "getrandom", path = "patches/getrandom-04/getrandom-0.4.3-zos" }
mio = { path = "patches/mio/mio-1.2.2-zos" }
socket2 = { path = "patches/socket2/socket2-0.6.5-zos" }
rustix = { path = "patches/rustix/rustix-1.1.4-zos" }
fs2 = { path = "patches/fs2/fs2-0.4.3-zos" }
tokio = { path = "patches/tokio/tokio-1.53.1-zos" }

# Crates that declare rust-version > 1.86 but compile fine with 1.86
# (rust-version field stripped, no code changes)
ignore = { path = "patches/rust-version-strips/ignore-0.4.33" }
cargo-util = { path = "patches/rust-version-strips/cargo-util-0.2.30" }
globset = { path = "patches/rust-version-strips/globset-0.4.20" }
home = { path = "patches/rust-version-strips/home-0.5.12" }
```

## `.cargo/config.toml` — cross-compilation settings

```toml
[target.s390x-ibm-zos]
linker = "s390x-ibm-zos-cc"
rustflags = [
    # Disable s390x SIMD/vector: LLVM z/OS backend cannot lower vector
    # instructions at opt-level >= 2 (toml_parser simd, zerocopy, etc.)
    "-C", "target-feature=-vector",
]
```

## RUSTFLAGS — proc-macro injection

Proc-macro crates (`.so` files) must be pre-built natively and injected via
`--extern` because Cargo 1.86 cannot cross-compile proc-macros:

```sh
# Build natively first:
cargo build  # (host build)

# Then cross-check with --extern for each proc-macro:
export RUSTFLAGS="-C target-feature=-vector \
  --extern futures_macro=target/debug/deps/libfutures_macro-*.so \
  --extern serde_derive=target/debug/deps/libserde_derive-*.so \
  --extern thiserror_impl=target/debug/deps/libthiserror_impl-*.so \
  --extern tracing_attributes=target/debug/deps/libtracing_attributes-*.so \
  --extern tokio_macros=target/debug/deps/libtokio_macros-*.so \
  --extern pin_project_internal=target/debug/deps/libpin_project_internal-*.so \
  --extern uv_macros=target/debug/deps/libuv_macros-*.so \
  --extern munge_macro=target/debug/deps/libmunge_macro-*.so \
  --extern ptr_meta_derive=target/debug/deps/libptr_meta_derive-*.so \
  --extern ref_cast_impl=target/debug/deps/libref_cast_impl-*.so \
  --extern scroll_derive=target/debug/deps/libscroll_derive-*.so \
  --extern zerovec_derive=target/debug/deps/libzerovec_derive-*.so \
  --extern zerofrom_derive=target/debug/deps/libzerofrom_derive-*.so \
  --extern bytecheck_derive=target/debug/deps/libbytecheck_derive-*.so \
  --extern yoke_derive=target/debug/deps/libyoke_derive-*.so \
  --extern displaydoc=target/debug/deps/libdisplaydoc-*.so"

cargo check --target s390x-ibm-zos -p uv
```

## Remaining blockers (as of 2026-08-09)

1. **`uv-platform-tags`**: needs `Os::Zos` variant with `os390_{major}_{release}_{version}` tag format
2. **`goblin`**: ELF parsing fails to compile for z/OS — needs `default-features = false` or a z/OS stub
3. **`ring`**: C build script probes compiler — needs z/OS target recognition or disable TLS
4. **`bzip2-sys`**, **`zstd-sys`**, **`lzma-sys`**: same build script issue as ring
5. **`dirent.d_extra`**: `*mut c_void` field makes libc `dirent` non-`Send`/`Sync` — needs `unsafe impl`
6. **`rustix` `VDSUSP`**: BSD-only termios constant referenced without z/OS exclusion
7. **`rustix` `ST_NODEV`**: Linux statvfs flag missing from libc-zos
