# uvport — z/OS port of [uv](https://github.com/astral-sh/uv)

uv is an extremely fast Python package and project manager written in Rust.
This port provides a z/OS (s390x-ibm-zos) binary built from **uv 0.8.13** — the last
0.8.x release that requires `rust-version = "1.86"`, matching the IBM z/OS Rust toolchain.

## Installation (via zopen)

```sh
zopen install uv
```

## Manual installation

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

Note: `uv python install` (downloading managed Python builds) is not supported on z/OS.
Use the IBM Open Enterprise SDK for Python available at `/usr/lpp/IBM/cyp/`.

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

### Crate patches required

Several upstream crates lack z/OS support. Patches are maintained in the
[`patches/`](./patches/) directory and applied via `[patch.crates-io]` in `Cargo.toml`.

| Crate | Version | Patch | Why |
|---|---|---|---|
| **libc** | 0.2.186 | [`patches/libc/`](./patches/libc/) | Add z/OS socket, termios, fs, dirent symbols |
| **mio** | 1.2.2 | [`patches/mio/`](./patches/mio/) | Add `target_os = "zos"` to poll selector, pipe waker, tcp/net |
| **socket2** | 0.6.5 | [`patches/socket2/`](./patches/socket2/) | `IovLen`, `ip_mreqn`, `tv_usec_pad` for z/OS |
| **rustix** | 1.1.4 | [`patches/rustix/`](./patches/rustix/) | `_Opcode`, errno codes, fs syscalls, `d_type` stub |
| **getrandom** | 0.2.17 | [`patches/getrandom-02/`](./patches/getrandom-02/) | `/dev/urandom` backend for z/OS |
| **getrandom** | 0.4.3 | [`patches/getrandom-04/`](./patches/getrandom-04/) | `/dev/urandom` backend + `libc` dep for z/OS |
| **tokio** | 1.53.1 | [`patches/tokio/`](./patches/tokio/) | `get_peer_cred` no-op (z/OS lacks `SO_PEERCRED`) |
| **fs2** | 0.4.3 | [`patches/fs2/`](./patches/fs2/) | `allocate()` fallback for z/OS |

Additionally these crates have their `rust-version` field stripped (no code changes):

| Crate | Version | Reason |
|---|---|---|
| **ignore** | 0.4.33 | declares `rust-version = "1.88"` |
| **cargo-util** | 0.2.30 | declares `rust-version = "1.94"` |
| **globset** | 0.4.20 | declares `rust-version = "1.88"` |
| **home** | 0.5.12 | declares `rust-version = "1.88"` |

### Still pending

The following blockers remain before a full build:

- **`uv-platform-tags`**: needs `Os::Zos` added to the platform tag enum
- **`goblin`**: ELF parsing gated on linux/macos/windows; needs z/OS stub
- **`ring`/`bzip2-sys`/`zstd-sys`/`lzma-sys`**: C build scripts need z/OS awareness
- **`*mut c_void` Send/Sync**: libc `dirent.d_extra` field needs `unsafe impl Send/Sync`

See [`patches/uv/`](./patches/uv/) for the in-progress Cargo.toml patches and
[`patches/README.md`](./patches/README.md) for detailed notes on each blocker.

## Upstream PRs

Patches being upstreamed:

- **getrandom**: <https://github.com/rust-random/getrandom/pull/859>
- **pyo3**: <https://github.com/PyO3/pyo3/pull/6314>
- mio, socket2, rustix, tokio PRs to be filed once full build is confirmed

## License

uv is MIT/Apache-2.0 licensed. See <https://github.com/astral-sh/uv/blob/main/LICENSE-MIT>.
