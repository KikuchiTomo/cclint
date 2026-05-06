#!/usr/bin/env bash
# Build a distribution tarball that bundles cclint and libclang.
#
# Usage:
#   scripts/dist.sh [<output-dir>]
#
# Output: dist/cclint-<version>-<os>-<arch>.tar.gz
#   bin/cclint
#   lib/libclang.{dylib,so,so.X}
#   LICENSE          (GPL-3.0)
#   LICENSE-LLVM.txt (Apache-2.0 with LLVM Exceptions)
#   README.md

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

OUT_DIR="${1:-dist}"
mkdir -p "$OUT_DIR"

VERSION="$(grep -m1 '^version' Cargo.toml | cut -d'"' -f2)"
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH_RAW="$(uname -m)"
case "$ARCH_RAW" in
  arm64|aarch64) ARCH=aarch64 ;;
  x86_64|amd64)  ARCH=x86_64 ;;
  *) echo "unsupported arch: $ARCH_RAW" >&2; exit 1 ;;
esac

case "$OS" in
  darwin) DYLIB_EXT="dylib" ;;
  linux)  DYLIB_EXT="so" ;;
  *) echo "unsupported OS: $OS" >&2; exit 1 ;;
esac

NAME="cclint-${VERSION}-${OS}-${ARCH}"
STAGE="$OUT_DIR/$NAME"
rm -rf "$STAGE"
mkdir -p "$STAGE/bin" "$STAGE/lib"

echo "==> building cclint (release)"
cargo build --release --bin cclint

cp target/release/cclint "$STAGE/bin/cclint"

echo "==> locating libclang"
LIBCLANG_SRC=""
if [ "$OS" = "darwin" ]; then
  for cand in \
    "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib" \
    "/opt/homebrew/opt/llvm/lib/libclang.dylib" \
    "/usr/local/opt/llvm/lib/libclang.dylib"; do
    if [ -f "$cand" ]; then LIBCLANG_SRC="$cand"; break; fi
  done
  if [ -z "$LIBCLANG_SRC" ]; then
    XCODE_LIB="$(dirname "$(dirname "$(xcrun --find clang)")")/lib"
    [ -f "$XCODE_LIB/libclang.dylib" ] && LIBCLANG_SRC="$XCODE_LIB/libclang.dylib"
  fi
else
  # Pick the highest available llvm-config-<N> on PATH.
  LLVM_CONFIG="$(ls /usr/bin/llvm-config-* /usr/local/bin/llvm-config-* 2>/dev/null | sort -V | tail -1 || true)"
  if [ -z "$LLVM_CONFIG" ]; then
    LLVM_CONFIG="$(command -v llvm-config 2>/dev/null || true)"
  fi
  if [ -n "$LLVM_CONFIG" ]; then
    LIBDIR="$($LLVM_CONFIG --libdir)"
    for cand in "$LIBDIR/libclang.so" "$LIBDIR/libclang.so.1" $LIBDIR/libclang-*.so.*; do
      if [ -f "$cand" ]; then LIBCLANG_SRC="$cand"; break; fi
    done
  fi
  if [ -z "$LIBCLANG_SRC" ]; then
    for cand in /usr/lib/x86_64-linux-gnu/libclang-*.so.* /usr/lib/aarch64-linux-gnu/libclang-*.so.* /usr/lib/llvm-*/lib/libclang.so*; do
      if [ -f "$cand" ]; then LIBCLANG_SRC="$cand"; break; fi
    done
  fi
fi

if [ -z "$LIBCLANG_SRC" ]; then
  echo "ERROR: libclang not found." >&2
  exit 1
fi
echo "    using $LIBCLANG_SRC"

# Copy the real file (deref symlinks). Use the canonical bundled name.
DEST_NAME="libclang.${DYLIB_EXT}"
cp -L "$LIBCLANG_SRC" "$STAGE/lib/$DEST_NAME"

echo "==> patching rpath"
if [ "$OS" = "darwin" ]; then
  install_name_tool -add_rpath "@executable_path/../lib" "$STAGE/bin/cclint" 2>/dev/null || true
  install_name_tool -id "@rpath/$DEST_NAME" "$STAGE/lib/$DEST_NAME" 2>/dev/null || true
else
  if command -v patchelf >/dev/null 2>&1; then
    patchelf --set-rpath '$ORIGIN/../lib' "$STAGE/bin/cclint"
  else
    echo "WARN: patchelf missing; rpath unset (LIBCLANG_PATH fallback still works)."
  fi
fi

cp LICENSE "$STAGE/LICENSE"
cp README.md "$STAGE/README.md"

# LLVM license file matching the bundled libclang.
LLVM_LICENSE_PATH=""
if [ "$OS" = "darwin" ]; then
  for cand in \
    "$(dirname "$LIBCLANG_SRC")/../LICENSE.TXT" \
    "$(dirname "$LIBCLANG_SRC")/LICENSE.TXT" \
    "/opt/homebrew/opt/llvm/LICENSE.TXT"; do
    [ -f "$cand" ] && LLVM_LICENSE_PATH="$cand" && break
  done
else
  for cand in \
    "/usr/share/doc/libclang-*/copyright" \
    "/usr/share/doc/llvm-*/copyright"; do
    f=$(ls $cand 2>/dev/null | head -1)
    [ -n "$f" ] && [ -f "$f" ] && LLVM_LICENSE_PATH="$f" && break
  done
fi
if [ -n "$LLVM_LICENSE_PATH" ]; then
  cp "$LLVM_LICENSE_PATH" "$STAGE/LICENSE-LLVM.txt"
else
  cat > "$STAGE/LICENSE-LLVM.txt" <<'EOF'
This distribution bundles libclang from the LLVM project, which is licensed
under the Apache License v2.0 with LLVM Exceptions.
See https://llvm.org/LICENSE.txt for the full license text.
EOF
fi

cat > "$STAGE/NOTICE" <<EOF
cclint $VERSION
GPL-3.0-or-later

Bundles libclang from the LLVM project (Apache-2.0 with LLVM Exceptions).
See LICENSE for cclint, LICENSE-LLVM.txt for libclang.
EOF

TARBALL="$OUT_DIR/${NAME}.tar.gz"
echo "==> packing $TARBALL"
(cd "$OUT_DIR" && tar -czf "${NAME}.tar.gz" "$NAME")
echo "==> done: $TARBALL"

echo "==> smoke test"
"$STAGE/bin/cclint" --version
