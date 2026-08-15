# uv 0.8.13 z/OS patches

## 0001-zos-platform-support.patch

Adds z/OS recognition to uv's platform detection stack so that:
- `uv venv`, `uv pip`, `uv add` work on z/OS
- Correct wheel tags (`os390_29_00_8561`) are generated for Python 3.12/3.13/3.14
- TLS connections to PyPI work (via rustls/ring big-endian patches)

### Files changed

| File | Change |
|------|--------|
| `crates/uv-platform-tags/src/platform.rs` | Add `Zos { release }` OS variant |
| `crates/uv-platform-tags/src/platform_tag.rs` | Add `Zos { release_arch }` tag; format as `os390_<release_arch>` |
| `crates/uv-platform-tags/src/tags.rs` | Map `Os::Zos` → `PlatformTag::Zos`; convert `29.00-8561` → `29_00_8561` |
| `crates/uv-platform/src/os.rs` | `Os::Zos` from `uv_platform_tags::Os::Zos` |
| `crates/uv-python/python/get_interpreter_info.py` | Handle both `os390` (3.12/3.13) and `zos` (3.14+) from `sysconfig.get_platform()` |
| `crates/uv-torch/src/backend.rs` | Skip torch backend on z/OS |

### Python 3.14 note (`zos` platform)

Python 3.14 changed `sysconfig.get_platform()` from `os390-29.00-8561` to just `zos`
(no version/build suffix). The patch handles this by:
1. Matching `operating_system in ("os390", "zos")` — both spellings handled in one branch
2. When `version_arch` is empty (3.14 case), reconstructing from `platform.release()` + `platform.machine()`
3. Passing `release = "29.00-8561"` to uv's OS struct regardless of Python version

This gives identical wheel tags (`os390_29_00_8561`) for all three Python versions.
