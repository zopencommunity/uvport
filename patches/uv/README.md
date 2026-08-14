# uv 0.8.13 z/OS patches

## 0001-zos-platform-support.patch

Adds z/OS recognition to uv's platform detection stack so that:

1. `get_interpreter_info.py` — Python interpreter query script maps
   `sysconfig.get_platform()` result `"os390-<ver>-<build>"` to a recognised
   operating system (`{"name": "zos", "release": "<ver>-<build>"}`) and sets
   `architecture = "s390x"` (the build suffix is not an arch).

2. `uv-platform-tags` — adds `Os::Zos { release: String }` variant with:
   - `serde(tag = "name")` deserialization from `{"name": "zos", ...}`
   - `PlatformTag::Zos { release_arch }` matching the `os390_29_00_8561` format
   - `FromStr` parser for `os390_*` platform tag strings
   - Platform-tag generation: `os390_<release_normalised>` (dots/dashes → `_`)

3. `uv-platform` — adds `Os::Zos` arm in `From<uv_platform_tags::Os>` mapping to
   `target_lexicon::OperatingSystem::Zos`.

4. `uv-torch` — adds `Os::Zos { .. }` to the CPU-fallback arms in
   `TorchAccelerator::index_urls()`.

## Cargo.toml — `[patch.crates-io]` additions

Add the following to uv's workspace `Cargo.toml` (merged with the existing
`[patch.crates-io]` block):

```toml
[patch.crates-io]
# z/OS crate patches
libc = { path = "patches/libc/libc-0.2.186-zos" }
errno = { path = "patches/errno/errno-0.3.14-zos" }
getrandom = { path = "patches/getrandom-02/getrandom-0.2.17-zos" }
"getrandom04" = { package = "getrandom", path = "patches/getrandom-04/getrandom-0.4.3-zos" }
mio = { path = "patches/mio/mio-1.2.2-zos" }
socket2 = { path = "patches/socket2/socket2-0.6.5-zos" }
rustix = { path = "patches/rustix/rustix-1.1.4-zos" }
fs2 = { path = "patches/fs2/fs2-0.4.3-zos" }
tokio = { path = "patches/tokio/tokio-1.53.1-zos" }
nix = { path = "patches/nix-030/nix-0.30.1-zos" }
thiserror = { path = "patches/thiserror/thiserror-2.0.12-zos" }
ring = { path = "patches/ring/ring-0.17.14-zos" }
sys-info = { path = "patches/sys-info/sys-info-0.9.1-zos" }
target-lexicon = { path = "patches/target-lexicon/target-lexicon-0.13.2-zos" }
```

## `.cargo/config.toml` — cross-compilation settings

```toml
[target.s390x-ibm-zos]
linker = "s390x-ibm-zos-cc"
rustflags = ["-C", "target-feature=-vector"]

[profile.zos]
inherits = "release"
opt-level = "z"
debug = 0
lto = false
strip = true
codegen-units = 1

[resolver]
incompatible-rust-versions = "allow"
```

## `libzos_fdopendir.a` stub (link-time injection)

z/OS's system `fdopendir` (`@@FDODIR`) is EBCDIC-only and incompatible with
ASCII-mode readdir (`@@A00372`). A stub `libzos_fdopendir.a` implements
`fdopendir` using `/proc/self/fd/<fd>` → `readlink` → `opendir`, which always
returns an ASCII-compatible `DIR*`. The stub is injected at link time by the
cross-compilation server (before any `.x` side-deck files).

See: `cross/server.py` in the rust-scripts build infrastructure.

## `mio` pipe.rs fix (patches/mio/mio-1.2.2-zos)

z/OS does not have `pipe2(2)`. The mio `new_raw()` function must use the
`pipe(2) + fcntl` fallback path. Add `target_os = "zos"` to the `pipe+fcntl`
`#[cfg(any(...))]` block in `src/sys/unix/pipe.rs`.

## Build command

```sh
cargo build --profile zos --target s390x-ibm-zos -p uv --bin uv \
    --ignore-rust-version
```
