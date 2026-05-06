#!/usr/bin/env bash
# cclint installer. One-liner via curl.
#
# Usage:
#   curl -fsSL https://github.com/KikuchiTomo/cclint/raw/main/scripts/install.sh | bash
#
# Environment:
#   CCLINT_VERSION  Version to install (default: latest)
#   CCLINT_PREFIX   Install prefix (default: $HOME/.local)
#   CCLINT_REPO     Repository (default: KikuchiTomo/cclint)
#   CCLINT_YES      Skip the upgrade confirmation prompt

set -euo pipefail

REPO="${CCLINT_REPO:-KikuchiTomo/cclint}"
VERSION="${CCLINT_VERSION:-latest}"
PREFIX="${CCLINT_PREFIX:-$HOME/.local}"

red()   { printf '\033[31m%s\033[0m\n' "$*" >&2; }
green() { printf '\033[32m%s\033[0m\n' "$*"; }
info()  { printf '==> %s\n' "$*"; }

OS_RAW="$(uname -s)"
ARCH_RAW="$(uname -m)"

case "$OS_RAW" in
  Linux)  OS=linux ;;
  Darwin) OS=darwin ;;
  *) red "unsupported OS: $OS_RAW"; exit 1 ;;
esac

case "$ARCH_RAW" in
  x86_64|amd64)  ARCH=x86_64 ;;
  arm64|aarch64) ARCH=aarch64 ;;
  *) red "unsupported CPU: $ARCH_RAW"; exit 1 ;;
esac

info "platform: $OS / $ARCH"

need() {
  command -v "$1" >/dev/null 2>&1 || { red "$1 is required"; exit 1; }
}
need curl
need tar

if [ "$VERSION" = "latest" ]; then
  info "resolving latest release"
  RESP="$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>/dev/null || true)"
  VERSION="$(echo "$RESP" | grep -m1 '"tag_name"' | sed -E 's/.*"tag_name":[[:space:]]*"([^"]+)".*/\1/' || true)"
  if [ -z "$VERSION" ]; then
    red "no release found"
    red "  releases: https://github.com/$REPO/releases"
    red "  pin a version: CCLINT_VERSION=v0.1.0 ... | bash"
    exit 1
  fi
fi

# Detect existing install and prompt for upgrade.
EXISTING="$PREFIX/bin/cclint"
if [ -x "$EXISTING" ]; then
  CURRENT_VER="$("$EXISTING" --version 2>/dev/null | awk '{print $2}' || true)"
  TARGET_VER="${VERSION#v}"
  if [ -n "$CURRENT_VER" ]; then
    info "existing install: $EXISTING (v$CURRENT_VER)"
    if [ "$CURRENT_VER" = "$TARGET_VER" ]; then
      green "already at $VERSION; nothing to do."
      exit 0
    fi
    OLDER="$(printf '%s\n%s\n' "$CURRENT_VER" "$TARGET_VER" | sort -V | head -1)"
    if [ "$OLDER" = "$TARGET_VER" ] && [ "$CURRENT_VER" != "$TARGET_VER" ]; then
      red "warning: installed v$CURRENT_VER is newer than target v$TARGET_VER (downgrade)"
    else
      info "upgrade: v$CURRENT_VER -> v$TARGET_VER"
    fi
    if [ "${CCLINT_YES:-0}" != "1" ] && [ "${CCLINT_FORCE:-0}" != "1" ]; then
      printf "Proceed? [y/N]: "
      ANS=""
      if [ -r /dev/tty ]; then
        read -r ANS < /dev/tty || true
      else
        read -r ANS || true
      fi
      case "$ANS" in
        y|Y|yes|YES) ;;
        *) info "cancelled."; exit 0 ;;
      esac
    fi
  fi
fi

VERSION_NO_V="${VERSION#v}"
TARBALL="cclint-${VERSION_NO_V}-${OS}-${ARCH}.tar.gz"
URL="https://github.com/$REPO/releases/download/${VERSION}/${TARBALL}"

info "downloading $URL"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

if ! curl -fsSL "$URL" -o "$TMPDIR/$TARBALL"; then
  red "download failed: $URL"
  red "releases: https://github.com/$REPO/releases"
  exit 1
fi

if curl -fsSL "https://github.com/$REPO/releases/download/${VERSION}/SHA256SUMS" -o "$TMPDIR/SHA256SUMS" 2>/dev/null; then
  info "verifying checksum"
  EXPECTED="$(grep "$TARBALL" "$TMPDIR/SHA256SUMS" | awk '{print $1}')"
  if [ -n "$EXPECTED" ]; then
    if command -v sha256sum >/dev/null 2>&1; then
      ACTUAL="$(sha256sum "$TMPDIR/$TARBALL" | awk '{print $1}')"
    else
      ACTUAL="$(shasum -a 256 "$TMPDIR/$TARBALL" | awk '{print $1}')"
    fi
    if [ "$EXPECTED" != "$ACTUAL" ]; then
      red "checksum mismatch: expected=$EXPECTED actual=$ACTUAL"
      exit 1
    fi
    green "checksum OK"
  fi
fi

info "extracting"
tar -xzf "$TMPDIR/$TARBALL" -C "$TMPDIR"
EXTRACTED="$TMPDIR/cclint-${VERSION_NO_V}-${OS}-${ARCH}"

mkdir -p "$PREFIX/bin" "$PREFIX/lib" "$PREFIX/share/cclint"

info "installing to $PREFIX"
install -m 0755 "$EXTRACTED/bin/cclint" "$PREFIX/bin/cclint"
cp -f "$EXTRACTED/lib/"libclang* "$PREFIX/lib/"
cp -f "$EXTRACTED/LICENSE"          "$PREFIX/share/cclint/LICENSE"
cp -f "$EXTRACTED/LICENSE-LLVM.txt" "$PREFIX/share/cclint/LICENSE-LLVM.txt" 2>/dev/null || true
cp -f "$EXTRACTED/NOTICE"           "$PREFIX/share/cclint/NOTICE" 2>/dev/null || true

# rpath in the tarball points at @executable_path/../lib (macOS) or
# $ORIGIN/../lib (Linux), so $PREFIX/bin/cclint finds $PREFIX/lib/libclang.*.

if "$PREFIX/bin/cclint" --version >/dev/null 2>&1; then
  green "installed: $PREFIX/bin/cclint $("$PREFIX/bin/cclint" --version)"
else
  red "startup check failed; try LIBCLANG_PATH=$PREFIX/lib"
  exit 1
fi

case ":$PATH:" in
  *":$PREFIX/bin:"*) ;;
  *)
    cat <<EOF

note: $PREFIX/bin is not on PATH. Add the following to ~/.bashrc or ~/.zshrc:

  export PATH="$PREFIX/bin:\$PATH"
EOF
    ;;
esac
