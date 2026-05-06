# cclint (customizable c++ linter)

C++ 用の linter です．AST は libclang から取得し，ルールは Lua で書きます．

## 対応環境

- Linux x86_64 / aarch64
- macOS aarch64 (Apple Silicon)

Intel mac (x86_64) は事前ビルドバイナリを提供していません．`CONTRIBUTING.md` を参照してソースからビルドしてください．

## インストール

```bash
curl -fsSL https://raw.githubusercontent.com/KikuchiTomo/cclint/main/scripts/install.sh | bash
```

`$HOME/.local/bin/cclint` に入ります．インストール先を変えたいときは `CCLINT_PREFIX` で指定してください．

```bash
CCLINT_PREFIX=/usr/local curl -fsSL https://raw.githubusercontent.com/KikuchiTomo/cclint/main/scripts/install.sh | sudo bash
```

libclang は同梱されています．別途インストールは不要です．

## 使い方

`.cclint.toml` をプロジェクト直下に置いてください．

```toml
cpp_standard = "c++17"
include_patterns = ["src/**/*.cpp", "src/**/*.h"]
exclude_patterns = ["build/**"]

[[rules]]
path = "rules/private_prefix.lua"
```

実行は `cclint .` です．`--format json` で機械可読出力になります．

## ドキュメント

- `docs/api.md` — Lua API リファレンス
- `docs/rules.md` — 同梱サンプルルール一覧
- `CONTRIBUTING.md` — 開発者向け（ソースからのビルド方法など）

## ライセンス

GPL-3.0-or-later．詳細は `LICENSE` を確認してください．
