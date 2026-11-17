#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

CHECK_ONLY=false
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
TARGETS=()

usage() {
    cat <<EOF
Usage: $(basename "$0") [--check] [paths...]

Format C/C++ source and header files using clang-format.
If no paths are provided, defaults to src/ and tests/.

Options:
  --check       Check formatting without modifying files (exits non-zero on diffs)
  -h, --help    Show this help message

Environment Variables:
  CLANG_FORMAT  Path or name of clang-format binary (default: clang-format)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --check)
            CHECK_ONLY=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
        *)
            TARGETS+=("$1")
            shift
            ;;
    esac
done

if ! command -v "${CLANG_FORMAT}" >/dev/null 2>&1; then
    echo "Error: ${CLANG_FORMAT} is not installed or not in PATH." >&2
    exit 1
fi

if [[ ${#TARGETS[@]} -eq 0 ]]; then
    TARGETS=("${REPO_ROOT}/src" "${REPO_ROOT}/tests")
fi

CLANG_FORMAT_ARGS=(--style=file)
if [[ "${CHECK_ONLY}" == true ]]; then
    CLANG_FORMAT_ARGS+=(--dry-run --Werror)
else
    CLANG_FORMAT_ARGS+=(-i)
fi

find "${TARGETS[@]}" \
    -type f \( -name '*.cpp' -o -name '*.h' \) -print0 \
    | xargs -0 -r "${CLANG_FORMAT}" "${CLANG_FORMAT_ARGS[@]}"
