# cclint への貢献

## ソースからのビルド

### 必要なもの

- Rust stable (1.75 以上)
- libclang (ビルド時にも実行時にも必要)
- C 開発ツールチェーン

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

`make` が `LIBCLANG_PATH` を自動検出します．手動指定したい場合は環境変数で渡してください．

## 開発コマンド

```
make build           # リリースビルド
make run             # tests/cpp_fixtures に対して example ルールを実行
make test            # 全テスト
make test-rust       # cargo test のみ
make test-fixtures   # 違反検出 e2e
make test-fixtures-clean  # 誤検出ゼロ確認
make fmt             # cargo fmt
make lint            # cargo clippy
make dist            # 配布用 tarball を dist/ に作る
make install         # /usr/local/bin にインストール
make clean
```

## ディレクトリ構成

```
crates/
  cclint-cli/         バイナリ (clap)
  cclint-config/      .cclint.toml ローダ
  cclint-ast/         libclang ラッパ + OwnedNode
  cclint-script/      Lua バインディング
  cclint-diagnostic/  診断 + 整形出力
examples/rules/       サンプル Lua ルール
tests/cpp_fixtures/         違反コードでルールが発火するか確認する fixture
tests/cpp_fixtures_clean/   誤検出が無いか確認する健全コード fixture
scripts/
  dist.sh             配布 tarball 作成
  install.sh          curl 一発インストーラ
.github/workflows/
  ci.yml              ubuntu/macos マトリクスで test
  release.yml         タグ push でリリースビルド
```

## コミット

Conventional Commits で書いてください．

```
feat: 〜
fix: 〜
docs: 〜
refactor: 〜
test: 〜
chore: 〜
```

破壊的変更は `feat!:` または footer に `BREAKING CHANGE:` を入れてください．

## リリース

`git tag v0.1.0 && git push --tags` で `release.yml` が起動し，4 プラットフォーム
分の tarball を GitHub Release に publish します．
