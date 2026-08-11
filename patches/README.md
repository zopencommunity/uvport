# Patches for uv 0.8.13 z/OS port

All patches are generated with `diff -ruN <orig>/ <patched>/` against the
crates.io registry version of each crate.

To apply during build: set `[patch.crates-io]` entries in uv's `Cargo.toml`
pointing to local directories where the patches have been applied.

---

## libc — `patches/libc/libc-0.2.186-zos.patch`

**Why**: The IBM libc fork (`github.ibm.com/compiler/rust-libc`) already has
extensive z/OS support, but is missing symbols needed by uv's dependency tree.
We add:

- `ip_mreqn` struct (for socket2 multicast)
- `IP_HDRINCL`, `IP_RECVTOS`, `TCP_KEEPIDLE`, `IPV6_ADD_MEMBERSHIP`, `IPV6_DROP_MEMBERSHIP`
- `O_DSYNC`, `O_ASYNC` (stub = 0)
- `DT_REG`, `DT_DIR`, `DT_LNK`, `DT_BLK`, `DT_CHR`, `DT_SOCK`, `DT_FIFO`, `DT_UNKNOWN` (for rustix dir iterator)
- `POSIX_FADV_*` constants
- High baud rates: `B57600` through `B4000000`
- Extended termios: `ECHOCTL`, `ECHOPRT`, `ECHOKE`, `EXTPROC`, `CRTSCTS`, `CMSPAR`, `IUTF8`, `IMAXBEL`
- Extra VXXX indices: `VDISCARD`, `VEOL2`, `VLNEXT`, `VREPRINT`, `VSWTC`, `VWERASE`
- `fsid_t` struct
- `DT_*` constants (dirent type, z/OS dirent has no `d_type` so always `DT_UNKNOWN`)
- Functions: `preadv`, `pwritev`, `futimens`, `posix_fadvise`, `posix_fallocate`, `major`, `minor`, `makedev`

---

## mio — `patches/mio/mio-1.2.2-zos.patch`

**Why**: mio 1.2.2 routes unknown unix targets to a compile error. z/OS uses:
- **Selector**: `poll(2)`-based (same as AIX/NTO/Haiku)
- **Waker**: `pipe(2)`-based (same as AIX/Dragonfly/OpenBSD)

Changes:
- `src/sys/unix/mod.rs`: add `target_os = "zos"` to poll selector, pipe waker, and pipe module cfg lists
- `src/sys/unix/net.rs`: add `target_os = "zos"` to `SOCK_NONBLOCK | SOCK_CLOEXEC` support; add `sin_len`/`sin6_len` (z/OS has BSD-style socket structs)
- `src/sys/unix/tcp.rs`: add `target_os = "zos"` to `accept4()` support list
- `src/sys/unix/selector/poll.rs`: add `AsFd`/`AsRawFd` impl for `Selector` (required by mio's `Registry`)

---

## socket2 — `patches/socket2/socket2-0.6.5-zos.patch`

**Why**: socket2 has no z/OS case for `IovLen` type or struct initializers.

Changes:
- `src/sys/unix.rs`: add `target_os = "zos"` to `IovLen = c_int` block
- `src/sys/unix.rs`: add `#[cfg(target_os = "zos")] tv_usec_pad: 0` in `into_timeval()` struct literals (z/OS `timeval` has a padding field)

---

## rustix — `patches/rustix/rustix-1.1.4-zos.patch`

**Why**: rustix 1.1.4 has no z/OS support at all.

Changes:
- `src/ioctl/mod.rs`: add z/OS to `_Opcode = c_int` block (same as AIX/Solaris)
- `src/backend/libc/io/errno.rs`: add `target_os = "zos"` to exclusion lists for ~40 Linux-specific errno codes (`ECHRNG`, `EL2*`, `EL3*`, `ELIB*`, `EHWPOISON`, etc.)
- `src/backend/libc/fs/types.rs`: add z/OS to exclusion lists for `FALLOC_FL_*`, `O_DSYNC`, `O_ASYNC`; add z/OS to `DT_*` usage (stub `DT_UNKNOWN`)
- `src/backend/libc/fs/dir.rs`: handle missing `d_type` field on z/OS dirent; always return `DT_UNKNOWN`
- `src/backend/libc/fs/makedev.rs`: add z/OS to unsafe makedev/major/minor path (same as AIX)
- `src/backend/libc/fs/syscalls.rs`: add z/OS to exclusion lists (aix pattern)
- `src/backend/libc/io/syscalls.rs`: z/OS now has preadv/pwritev (added to libc)
- `src/termios/types.rs`: add z/OS to exclusion lists for aix-like constants

---

## getrandom 0.2 — `patches/getrandom-02/getrandom-0.2.17-zos.patch`

**Why**: getrandom 0.2 has no `/dev/urandom` backend for z/OS.

Changes:
- `src/lib.rs`: add `target_os = "zos"` to the `use_file` backend group (same as AIX/Haiku/NTO)
- `src/util_libc.rs`: add `target_os = "zos"` to `__errno` branch

---

## getrandom 0.4 — `patches/getrandom-04/getrandom-0.4.3-zos.patch`

**Why**: getrandom 0.4 has no `/dev/urandom` backend or `libc` dependency for z/OS.

Changes:
- `src/backends.rs`: add `target_os = "zos"` to `use_file` backend group
- `src/utils/get_errno.rs`: add `target_os = "zos"` to `__errno` branch
- `Cargo.toml`: add z/OS to the `libc` conditional dependency cfg

---

## tokio — `patches/tokio/tokio-1.53.1-zos.patch`

**Why**: tokio's Unix socket credential API (`get_peer_cred`) has no z/OS implementation.
z/OS lacks `SO_PEERCRED`.

Changes:
- `src/net/unix/ucred.rs`: add `target_os = "zos"` to `impl_noproc` group (returns `ENOSYS`)

---

## fs2 — `patches/fs2/fs2-0.4.3-zos.patch`

**Why**: fs2's `allocate()` has no z/OS fallback.

Changes:
- `src/unix.rs`: add `target_os = "zos"` to the no-op `allocate()` group (same as OpenBSD/NetBSD)

---

## uv itself — `patches/uv/`

In-progress patches to uv's own crates:

### `uv-platform-tags` — needs `Os::Zos`

```rust
// crates/uv-platform-tags/src/platform.rs
pub enum Os {
    // ... existing variants ...
    Zos { major: u32, release: u32, version: String },
}
```

With corresponding tag generation: `os390_{major}_{release}_{version}`.

### `goblin` — ELF parsing stub for z/OS

goblin's ELF module is gated on `#[cfg(not(target_os = "windows"))]` but uses
`std::io::Read` trait bounds that fail to compile for z/OS. Needs a z/OS-aware stub
or to be excluded via `default-features = false`.

### Build scripts (ring, bzip2-sys, zstd-sys, lzma-sys)

These C libraries need their build scripts to recognize `target_os = "zos"` and
either provide z/OS-specific C sources or disable the feature. The cross-CC wrapper
at `/home/itodorov/rust_bld/toolchain/s390x-ibm-zos-cc` handles local clang compilation
for `-c`/`-E` probes but the full build requires the z/OS HTTP server.
