#!/bin/bash
# Inject PowerPC host proc macros omitted by Cargo 1.86 during z/OS cross-builds.
set -u
RUSTC="$1"
shift

is_zos=0
crate_src=""
for arg in "$@"; do
    [[ "$arg" == "s390x-ibm-zos" ]] && is_zos=1
    [[ -z "$crate_src" && "$arg" == *.rs ]] && crate_src="$arg"
done
[[ "$is_zos" == 0 ]] && exec "$RUSTC" "$@"

# Override this when using a target directory other than target/zos.
DEPS="${UV_HOST_PROC_MACRO_DEPS:-$PWD/target/zos/deps}"
extra=()
declare -A injected=()

has_extern() {
    local name="$1" arg
    shift
    for arg in "$@"; do
        [[ "$arg" == "$name="* ]] && return 0
    done
    return 1
}

for so in "$DEPS"/lib*.so; do
    [[ -e "$so" ]] || continue
    base=${so##*/lib}
    name=${base%%-*}
    [[ "$name" == "thiserror_impl" ]] && continue
    [[ -n "${injected[$name]:-}" ]] && continue
    if ! has_extern "$name" "$@"; then
        extra+=(--extern "$name=$so")
        injected[$name]=1
    fi
done

# uv has both thiserror major versions; select the metadata-compatible host DSO.
if ! has_extern thiserror_impl "$@" && [[ "$crate_src" == *thiserror-* ]]; then
    wanted=""
    [[ "$crate_src" == *thiserror-1.* ]] && wanted='thiserror-impl-1.'
    [[ "$crate_src" == *thiserror-2.* ]] && wanted='thiserror-impl-2.'
    if [[ -n "$wanted" ]]; then
        for so in "$DEPS"/libthiserror_impl-*.so; do
            strings "$so" 2>/dev/null | grep -q "$wanted" || continue
            extra+=(--extern "thiserror_impl=$so")
            break
        done
    fi
fi

exec "$RUSTC" "$@" "${extra[@]}"
