# cclint (customizable c++ linter)

C++ 用の linter です．AST は libclang から取得し，ルールは Lua で書きます．

## 対応環境

- Ubuntu 22.04 / 24.04
- macOS 14 / 15
- Rust stable (1.75 以上)
- libclang (実行時に dlopen で読み込みます)

## インストール

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential libclang-dev llvm-dev pkg-config
```

### macOS

```bash
xcode-select --install
# または
brew install llvm
```

`make` が `LIBCLANG_PATH` を自動検出します．手動で指定したいときは環境変数で渡してください．

## ビルドと実行

```bash
make build      # リリースビルド
make run        # tests/cpp_fixtures に対して実行
make test       # 全テスト (cargo test + 違反検出 + 誤検出ゼロ確認)
make install    # /usr/local/bin/cclint にインストール
make clean
```

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
- `CLAUDE.md` — 開発者向けガイド

## ライセンス

GPL-3.0-or-later．詳細は `LICENSE` を確認してください．
