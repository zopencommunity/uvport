# uvport — z/OS port of [uv](https://github.com/astral-sh/uv)

uv is an extremely fast Python package and project manager written in Rust.
This port targets **z/OS (s390x-ibm-zos)**, built from **uv 0.8.13** — the last
0.8.x release with `rust-version = "1.86"`, matching the IBM z/OS Rust toolchain.

## Status

| Component | Status |
|-----------|--------|
| All Rust crates compile | ✅ |
| `uv` binary | ✅ Linked and working |
| `uv --version` | ✅ `uv 0.8.13` |
| `uv init` | ✅ |
| `uv run` | ✅ |
| `uv add` (local wheel) | ✅ |
| `uv python list` | ✅ |
| PyPI network (TLS) | ⚠️ z/OS TLS stack limitation |

## Installation (via zopen)

> Once the binary is available:

```sh
zopen install uv
```

## Manual installation

> Once release assets are published:

```sh
curl -L -o uv https://github.com/zopencommunity/uvport/releases/download/v0.8.13/uv
chtag -b uv && chmod +x uv
curl -L -o uvx https://github.com/zopencommunity/uvport/releases/download/v0.8.13/uvx
chtag -b uvx && chmod +x uvx
```

## Usage on z/OS

uv auto-detects the IBM Python installation. Set once in your environment:

```sh
export UV_PYTHON=/usr/lpp/IBM/cyp/v3r12/pyz/bin/python3
export UV_PYTHON_PREFERENCE=only-system   # never attempt to download managed Python
```

Then use uv normally:

```sh
uv venv .venv
uv pip install fastmcp
uv run my_script.py
uv add requests
uv sync
```

> **Note**: `uv python install` (downloading managed Python builds) is not supported on z/OS.
> Use the IBM Open Enterprise SDK for Python at `/usr/lpp/IBM/cyp/`.

## Version pinning rationale

| uv version | rust-version | Notes |
|---|---|---|
| 0.8.0–0.8.13 | 1.86 | ✅ Our target — matches z/OS Rust toolchain |
| 0.8.14–0.8.18 | 1.87 | ❌ |
| 0.8.19+ | 1.88 | ❌ |

## Cross-compilation

uv 0.8.13 is cross-compiled on Linux-on-Power (ppc64le) using an IBM Rust toolchain
targeting `s390x-ibm-zos`. The build infrastructure lives in:
<https://github.ibm.com/compiler/rust-scripts> branch `itodorov/zos-cross-compile-setup`

### Architecture

```
LoP (Linux ppc64le)                   z/OS (s390x)
─────────────────────                 ─────────────
rustc 1.86 (nightly)
  + s390x-ibm-zos target    ──HTTP──► Flask server
  + ibm-clang wrapper                 ibm-clang + /bin/ld
  + llvm-ar (GOFF archives)           builds .so / executables
```

The `cross/` directory in rust-scripts contains:
- `client.py` — intercepts `cc`/`ar` calls from cargo, sends to z/OS server
- `server.py` — Flask server on z/OS, runs ibm-clang, caches objects
- `s390x-ibm-zos-cc` — cc wrapper that handles LTO, rlib extraction, GOFF

### Proc-macro workaround

Cargo 1.86 cannot execute z/OS `.so` proc-macro crates on a Linux host.
The workaround: pre-build all proc-macros natively for `powerpc64le-unknown-linux-gnu`
and inject them via `RUSTFLAGS='--extern crate=path.so ...'`.

Affected proc-macros for uv 0.8.13:
`rkyv_derive`, `clap_derive`, `miette_derive`, `schemars_derive`,
`thiserror_impl`, `tracing_attributes`, `async_trait`, `pin_project_internal`,
`tokio_macros`, `serde_derive`, `ref_cast_impl`, `dyn_clone_derive`, and more.

### Crate patches

All 30+ crate patches are in [`patches/`](./patches/). Key patches:

| Crate | Version | Why |
|---|---|---|
| **libc** | 0.2.186 | IBM fork with full z/OS `src/unix/zos/` module |
| **mio** | 1.2.2 | Poll selector + pipe waker for z/OS |
| **socket2** | 0.6.5 | `IovLen`, `ip_mreqn`, `tv_usec_pad` |
| **rustix** | 1.1.4 | Full z/OS port (ioctl, errno, fs, dir, termios) |
| **getrandom** | 0.2.17 | `/dev/urandom` backend |
| **getrandom** | 0.4.3 | `/dev/urandom` backend + libc dep |
| **tokio** | 1.53.1 | `get_peer_cred` no-op (no `SO_PEERCRED`) |
| **ring** | 0.17.14 | C headers, `__int128_t`, GOFF arch detection |
| **ctrlc** | 3.4.7 | pipe-based waker (no POSIX semaphores) |
| **nix** | 0.30.1 / 0.31.3 | errno, signal, time fixes |
| **rkyv** | 0.8.11 | Auto-enable `big_endian` on s390x |
| **target-lexicon** | 0.13.2 | Add `OperatingSystem::Zos` |
| **zeroize** | 1.8.1 | Portable memory barrier for s390x |
| **filetime** | 0.2.25 | `futimens`/`utimes` for z/OS |
| **jobserver** | 0.1.33 | Pipe-based jobserver path |
| **bzip2-sys** | 0.1.13 | Use pre-built static lib |
| **zstd-sys** | 2.0.15 | Use pre-built static lib |
| **lzma-sys** | 0.1.20 | Use pre-built static lib |
| **thiserror** | v1→v2 | All v1 users patched to use v2 |
| **ignore** | 0.4.33 | Strip `rust-version`; fix `let` chains |
| **globset** | 0.4.20 | Strip `rust-version` |
| **cargo-util** | 0.2.30 | Strip `rust-version`; paths fix |
| **home** | 0.5.12 | Strip `rust-version` |
| **fs2** | 0.4.3 | No-op `allocate()` |
| **sys-info** | 0.9.1 | z/OS stub |
| **rust-netrc** | 0.1.2 | Minor compat |

See [`patches/README.md`](./patches/README.md) for per-patch details.

## Upstream PRs

Patches being upstreamed:

| Crate | PR |
|---|---|
| getrandom | <https://github.com/rust-random/getrandom/pull/859> |
| pyo3 | <https://github.com/PyO3/pyo3/pull/6314> |
| mio, socket2, rustix, tokio, nix | To be filed once `uv` binary links |

## License

uv is MIT/Apache-2.0 licensed. See <https://github.com/astral-sh/uv/blob/main/LICENSE-MIT>.
