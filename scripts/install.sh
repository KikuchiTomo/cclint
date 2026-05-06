#!/usr/bin/env bash
# cclint インストーラ．curl 一発で入れるためのスクリプト．
#
# 使い方:
#   curl -fsSL https://raw.githubusercontent.com/tomokikuchi/cclint/main/scripts/install.sh | bash
#
# 環境変数:
#   CCLINT_VERSION  インストールするバージョン (デフォルト: latest)
#   CCLINT_PREFIX   インストール先 (デフォルト: $HOME/.local)
#   CCLINT_REPO     リポジトリ (デフォルト: tomokikuchi/cclint)

set -euo pipefail

REPO="${CCLINT_REPO:-tomokikuchi/cclint}"
VERSION="${CCLINT_VERSION:-latest}"
PREFIX="${CCLINT_PREFIX:-$HOME/.local}"

red()   { printf '\033[31m%s\033[0m\n' "$*" >&2; }
green() { printf '\033[32m%s\033[0m\n' "$*"; }
info()  { printf '==> %s\n' "$*"; }

# OS / Arch 検出
OS_RAW="$(uname -s)"
ARCH_RAW="$(uname -m)"

case "$OS_RAW" in
  Linux)  OS=linux ;;
  Darwin) OS=darwin ;;
  *) red "対応していない OS: $OS_RAW"; exit 1 ;;
esac

case "$ARCH_RAW" in
  x86_64|amd64)  ARCH=x86_64 ;;
  arm64|aarch64) ARCH=aarch64 ;;
  *) red "対応していない CPU: $ARCH_RAW"; exit 1 ;;
esac

# macOS では aarch64 ではなく aarch64 でビルドしているので統一
[ "$OS" = "darwin" ] && [ "$ARCH" = "aarch64" ] && ARCH=aarch64

info "platform: $OS / $ARCH"

# 必要コマンドの確認
need() {
  command -v "$1" >/dev/null 2>&1 || { red "$1 が必要です"; exit 1; }
}
need curl
need tar

# バージョン解決
if [ "$VERSION" = "latest" ]; then
  info "最新リリースを取得"
  RESP="$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>/dev/null || true)"
  VERSION="$(echo "$RESP" | grep -m1 '"tag_name"' | sed -E 's/.*"tag_name":[[:space:]]*"([^"]+)".*/\1/' || true)"
  if [ -z "$VERSION" ]; then
    red "リリースが見つかりません．"
    red "  リリース一覧: https://github.com/$REPO/releases"
    red "  バージョン指定: CCLINT_VERSION=v0.1.0 ... | bash"
    exit 1
  fi
fi

# 既存インストール検出 + バージョン比較
EXISTING="$PREFIX/bin/cclint"
if [ -x "$EXISTING" ]; then
  CURRENT_VER="$("$EXISTING" --version 2>/dev/null | awk '{print $2}' || true)"
  TARGET_VER="${VERSION#v}"
  if [ -n "$CURRENT_VER" ]; then
    info "既存インストール検出: $EXISTING (v$CURRENT_VER)"
    if [ "$CURRENT_VER" = "$TARGET_VER" ]; then
      green "既に $VERSION がインストール済みです．何もしません．"
      exit 0
    fi
    # バージョン比較 (sort -V で古い方を先頭に)
    OLDER="$(printf '%s\n%s\n' "$CURRENT_VER" "$TARGET_VER" | sort -V | head -1)"
    if [ "$OLDER" = "$TARGET_VER" ] && [ "$CURRENT_VER" != "$TARGET_VER" ]; then
      red "警告: 既存版 (v$CURRENT_VER) の方が新しい．ダウングレードします．"
    else
      info "アップデートあり: v$CURRENT_VER → v$TARGET_VER"
    fi
    # 確認プロンプト ($CCLINT_YES=1 でスキップ可能)
    if [ "${CCLINT_YES:-0}" != "1" ] && [ "${CCLINT_FORCE:-0}" != "1" ]; then
      printf "アップデートしますか? [y/N]: "
      ANS=""
      if [ -r /dev/tty ]; then
        read -r ANS < /dev/tty || true
      else
        read -r ANS || true
      fi
      case "$ANS" in
        y|Y|yes|YES) ;;
        *) info "キャンセルしました．"; exit 0 ;;
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
  red "ダウンロード失敗: $URL"
  red "リリース一覧: https://github.com/$REPO/releases"
  exit 1
fi

# SHA256 検証 (任意)
if curl -fsSL "https://github.com/$REPO/releases/download/${VERSION}/SHA256SUMS" -o "$TMPDIR/SHA256SUMS" 2>/dev/null; then
  info "checksum を検証"
  EXPECTED="$(grep "$TARBALL" "$TMPDIR/SHA256SUMS" | awk '{print $1}')"
  if [ -n "$EXPECTED" ]; then
    if command -v sha256sum >/dev/null 2>&1; then
      ACTUAL="$(sha256sum "$TMPDIR/$TARBALL" | awk '{print $1}')"
    else
      ACTUAL="$(shasum -a 256 "$TMPDIR/$TARBALL" | awk '{print $1}')"
    fi
    if [ "$EXPECTED" != "$ACTUAL" ]; then
      red "checksum 不一致: expected=$EXPECTED actual=$ACTUAL"
      exit 1
    fi
    green "checksum OK"
  fi
fi

info "extracting"
tar -xzf "$TMPDIR/$TARBALL" -C "$TMPDIR"
EXTRACTED="$TMPDIR/cclint-${VERSION_NO_V}-${OS}-${ARCH}"

# インストール先ディレクトリ
mkdir -p "$PREFIX/bin" "$PREFIX/lib" "$PREFIX/share/cclint"

info "installing to $PREFIX"
install -m 0755 "$EXTRACTED/bin/cclint" "$PREFIX/bin/cclint"
cp -f "$EXTRACTED/lib/"libclang* "$PREFIX/lib/"
cp -f "$EXTRACTED/LICENSE"          "$PREFIX/share/cclint/LICENSE"
cp -f "$EXTRACTED/LICENSE-LLVM.txt" "$PREFIX/share/cclint/LICENSE-LLVM.txt" 2>/dev/null || true
cp -f "$EXTRACTED/NOTICE"           "$PREFIX/share/cclint/NOTICE" 2>/dev/null || true

# rpath は tarball 内で `@executable_path/../lib` / `$ORIGIN/../lib` に設定済み．
# つまり `$PREFIX/bin/cclint` は `$PREFIX/lib/libclang.*` を見る．

# 動作確認
if "$PREFIX/bin/cclint" --version >/dev/null 2>&1; then
  green "インストール成功: $PREFIX/bin/cclint $("$PREFIX/bin/cclint" --version)"
else
  red "起動確認に失敗．LIBCLANG_PATH=$PREFIX/lib を試してください．"
  exit 1
fi

# PATH の確認
case ":$PATH:" in
  *":$PREFIX/bin:"*) ;;
  *)
    cat <<EOF

注意: $PREFIX/bin が PATH に入っていません．以下を ~/.bashrc / ~/.zshrc に追加してください．

  export PATH="$PREFIX/bin:\$PATH"
EOF
    ;;
esac
