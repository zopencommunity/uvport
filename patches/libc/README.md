# libc z/OS patch

The z/OS libc support comes from the IBM internal fork:

  **github.ibm.com/compiler/rust-libc** branch `zOS.0.2.169`

This fork is a superset of upstream libc 0.2.186 with comprehensive z/OS
`src/unix/zos/` module containing:

- Socket structs/constants (sockaddr_in, sockaddr_in6, ip_mreqn, etc.)
- Termios constants (high baud rates, ECHOCTL, CRTSCTS, etc.)
- Filesystem constants (O_DSYNC, O_ASYNC, POSIX_FADV_*, etc.)
- Dirent type constants (DT_REG, DT_DIR, etc.)
- Additional functions (preadv, pwritev, futimens, posix_fadvise, etc.)
- `fsid_t` struct

The Cargo.toml `[patch.crates-io]` entry points to a local checkout of this fork.
