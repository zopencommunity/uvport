# ring 0.17.14 z/OS Patches

Comprehensive patches for ring cryptographic library on z/OS (big-endian s390x).

## Issues Fixed

1. **`bn_mul_mont` missing** — Software Montgomery multiplication for z/OS
   (no assembly, no `__int128` in IBM XL C). Supports RSA-2048/4096.

2. **`bn_umult_lohi` missing** — Software 64×64→128 multiply for IBM XL C.

3. **P-256 fiat type mismatch** — 64-bit table vs 32-bit fiat implementation
   when `OPENSSL_64_BIT` is defined but `__int128` is unavailable.

4. **P-256 big-endian limb conversion** — `fiat_p256_from_words`/`to_words`
   must swap 32-bit halves on big-endian z/OS.

5. **Curve25519 field element type mismatch** — `fe` uses `uint64_t[5]` but
   fiat functions expect `uint32_t[10]` on z/OS.

6. **Curve25519 table constants** — 27 `#if defined(OPENSSL_64_BIT)` guards
   need `&& !defined(__MVS__)` to select 32-bit constants.

## Tested

- AES-128-GCM, AES-256-GCM, ChaCha20-Poly1305 ✓
- ECDH P-256, ECDH X25519 ✓
- HKDF-SHA256, SHA-256 ✓
- RSA-2048 sign+verify ✓
- ECDSA P-256 sign ✓
- Full TLS 1.3 handshake to pypi.org via rustls ✓
- `uv add requests` downloads from PyPI over HTTPS ✓
