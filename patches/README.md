# Patches for uv 0.8.13 z/OS port

All patches are generated with `diff -ruN <orig>/ <patched>/` against the
crates.io registry version of each crate (or the upstream git commit used by uv).

To reproduce the build, apply each patch to a local copy of the crate and add
`[patch.crates-io]` entries in uv's `Cargo.toml` pointing to those directories.

---

## Core OS / libc

### libc — `patches/libc/`

See [`patches/libc/README.md`](libc/README.md).  
Uses **github.ibm.com/compiler/rust-libc** branch `zOS.0.2.169` (IBM internal fork).  
No crates.io diff available; the fork adds a complete `src/unix/zos/` module.

---

## Async I/O

### mio — `patches/mio/mio-1.2.2-zos.patch`

**Why**: mio 1.2.2 has no z/OS selector/waker.  
**Changes**: add `target_os = "zos"` to poll selector, pipe waker, tcp/net; add `AsFd`/`AsRawFd` for `Selector`.

### tokio — `patches/tokio/tokio-1.53.1-zos.patch`

**Why**: `get_peer_cred` uses `SO_PEERCRED` which z/OS lacks.  
**Changes**: add `target_os = "zos"` to `impl_noproc` group in `ucred.rs`.

### async_http_range_reader — `patches/async-http-range-reader/async_http_range_reader-0.9.1-zos.patch`

**Why**: depends on thiserror v1; conflicts with our thiserror v2 redirect.  
**Changes**: `Cargo.toml` thiserror dependency → v2.

### rs-async-zip — `patches/rs-async-zip/rs-async-zip-git-zos.patch`

**Why**: depends on thiserror v1.  
**Changes**: `Cargo.toml` thiserror dependency → v2.

### reqwest-middleware — `patches/reqwest-middleware/reqwest-middleware-git-zos.patch`

**Why**: depends on thiserror v1.  
**Changes**: `Cargo.toml` thiserror dependency → v2.

### reqwest-retry — `patches/reqwest-retry/reqwest-retry-git-zos.patch`

**Why**: depends on thiserror v1 and has no z/OS awareness.  
**Changes**: `Cargo.toml` thiserror → v2; retry policy adapters.

---

## Networking / TLS

### socket2 — `patches/socket2/socket2-0.6.5-zos.patch`

**Why**: no z/OS `IovLen` type or `timeval` padding field.  
**Changes**: add `target_os = "zos"` to `IovLen = c_int` block; add `tv_usec_pad: 0` in `into_timeval()`.

### ring — `patches/ring/ring-0.17.14-zos.patch`

**Why**: ring's C sources use Linux/MSVC-specific headers and `__int128_t`.  
**Changes**: add z/OS guards in `build.rs`; fix `__int128_t`/`__uint128_t` typedefs in `crypto/internal.h`; add `s390x-ibm-zos` to GOFF arch detection.

### zeroize — `patches/zeroize/zeroize-1.8.1-zos.patch`

**Why**: zeroize uses x86/aarch64-specific volatile write intrinsics.  
**Changes**: add `src/barrier.rs` with a portable memory barrier; add z/OS to fallback path.

---

## Filesystem / System

### rustix — `patches/rustix/rustix-1.1.4-zos.patch`

**Why**: rustix 1.1.4 has no z/OS support.  
**Changes**: `_Opcode = c_int`; errno exclusions; `d_type` stub; `DT_*` constants; `makedev`; `preadv`/`pwritev`; termios exclusions.

### filetime — `patches/filetime/filetime-0.2.25-zos.patch`

**Why**: `utimes`/`futimens` path gated on Linux/macOS/etc.  
**Changes**: add `target_os = "zos"` to `futimens` and `utimes` call sites.

### fs2 — `patches/fs2/fs2-0.4.3-zos.patch`

**Why**: `allocate()` has no z/OS fallback.  
**Changes**: add `target_os = "zos"` to no-op `allocate()` group.

### jobserver — `patches/jobserver/jobserver-0.1.33-zos.patch`

**Why**: jobserver uses Linux-specific pipe/fd handling.  
**Changes**: add `target_os = "zos"` to POSIX pipe-based jobserver path.

### sys-info — `patches/sys-info/sys-info-0.9.1-zos.patch`

**Why**: `build.rs` and `lib.rs` don't recognize z/OS.  
**Changes**: add z/OS to `build.rs` (skip C compilation); add stub `os_release()` in `lib.rs`.

### bzip2-sys — `patches/bzip2-sys/bzip2-sys-0.1.13-zos.patch`

**Why**: `build.rs` uses pre-built static lib path for z/OS.  
**Changes**: add z/OS branch to skip cmake; use `/tmp/zos-sysroot/bzip2/lib/libbz2.a`.

### zstd-sys — `patches/zstd-sys/zstd-sys-2.0.15-zos.patch`

**Why**: `build.rs` uses pre-built static lib path for z/OS.  
**Changes**: add z/OS branch; use `/tmp/zos-sysroot/zstd/lib/libzstd.a`.

### lzma-sys — `patches/lzma-sys/lzma-sys-0.1.20-zos.patch`

**Why**: `build.rs` uses pre-built static lib path for z/OS.  
**Changes**: add z/OS branch; use `/tmp/zos-sysroot/lzma/lib/liblzma.a`.

---

## Entropy / Randomness

### getrandom 0.2 — `patches/getrandom-02/getrandom-0.2.17-zos.patch`

**Why**: no `/dev/urandom` backend for z/OS.  
**Changes**: add `target_os = "zos"` to `use_file` backend group; fix `__errno` in `util_libc.rs`.

### getrandom 0.4 — `patches/getrandom-04/getrandom-0.4.3-zos.patch`

**Why**: no `/dev/urandom` backend or `libc` dep for z/OS.  
**Changes**: add `target_os = "zos"` to `use_file` group; fix `__errno`; add `libc` dep in `Cargo.toml`.

---

## Cryptography / Hashing

### ring — see above.

---

## IPC / Signals

### ctrlc — `patches/ctrlc/ctrlc-3.4.7-zos.patch`

**Why**: ctrlc uses POSIX semaphores (`sem_t`) which z/OS doesn't support.  
**Changes**: replace semaphore-based signalling with `pipe(2)` approach for `target_os = "zos"`.

### nix 0.30 — `patches/nix-030/nix-0.30.1-zos.patch`

**Why**: nix 0.30.1 has partial z/OS support (errno, signal, time need fixes).  
**Changes**: errno exclusion list; signal constants; `TimeSpec`/`TimeVal` conversions.

### nix 0.31 — `patches/nix-031/nix-0.31.3-zos.patch`

**Why**: nix 0.31.3 same issues.  
**Changes**: same errno/signal/time fixes adapted for 0.31 API.

---

## Serialization

### rkyv — `patches/rkyv/rkyv-0.8.11-zos.patch`

**Why**: rkyv requires `big_endian` feature on big-endian targets but doesn't auto-detect.  
**Changes**: add `build.rs` that auto-enables `big_endian` cfg when `CARGO_CFG_TARGET_ENDIAN=big`.

---

## Platform / Target Detection

### target-lexicon — `patches/target-lexicon/target-lexicon-0.13.2-zos.patch`

**Why**: `s390x-ibm-zos` not recognized as a valid target triple.  
**Changes**: add `OperatingSystem::Zos`; add `s390x-ibm-zos` parsing in `Triple::from_str`.

---

## Misc

### rust-netrc — `patches/rust-netrc/rust-netrc-0.1.2-zos.patch`

**Why**: minor z/OS path compatibility.  
**Changes**: minimal `Cargo.toml` adjustment.

### ignore — `patches/ignore/ignore-0.4.33-zos.patch`

**Why**: `rust-version = "1.88"` in `Cargo.toml` blocks build; `src/incremental.rs` uses
`&& let` chains (stabilized in 1.88).  
**Changes**: remove `rust-version`; rewrite two `let` chains for 1.86 compat.

### globset — `patches/globset/globset-0.4.20-zos.patch`

**Why**: `rust-version = "1.88"`.  
**Changes**: remove `rust-version` from `Cargo.toml`.

### cargo-util — `patches/cargo-util/cargo-util-0.2.30-zos.patch`

**Why**: `rust-version = "1.94"` and minor path handling.  
**Changes**: remove `rust-version`; add z/OS to `paths.rs`.

### home — `patches/home/home-0.5.12-zos.patch`

**Why**: `rust-version = "1.88"`.  
**Changes**: remove `rust-version` from `Cargo.toml`.
