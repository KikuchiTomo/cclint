# Claude Code 開発ガイド (cclint)

## プロジェクト概要

**cclint** — libclang + Lua の C++ linter。

- 言語: Rust (stable)
- AST: libclang を内部組み込み (`clang` crate FFI)
- ルール記述: Lua 5.4 (`mlua` crate, vendored)
- 設定: `.cclint.toml`

## ディレクトリ構成

```
crates/
  cclint-cli/         # バイナリ (clap)
  cclint-config/      # .cclint.toml ローダ
  cclint-ast/         # libclang ラッパ + OwnedNode
  cclint-script/      # Lua バインディング
  cclint-diagnostic/  # 診断データ + codespan-reporting 出力
examples/rules/       # サンプル Lua ルール
tests/cpp_fixtures/   # e2e テスト用の C++ プロジェクト
Makefile              # build / run / test / install / clean
```

## 開発コマンド

```
make build          # リリースビルド
make test           # cargo test + fixture 検出 e2e
make test-rust      # cargo test のみ
make test-fixtures  # fixture の e2e のみ
make fmt / lint
make install / uninstall
make clean
```

## 設計上の不変条件

- **デフォルトでルール 0**。組込ルールは追加しない。
- **すべての Lua ルール名・メッセージは UTF-8**。日本語 OK。
- **AST ノードは値型 `OwnedNode`** に変換してから Lua へ渡す。
  libclang の `Entity` を直接 Lua に渡さないこと (寿命管理が破綻するため)。
- **clang 診断はそのまま `Diagnostic` として流す**。構文エラーが見えなくならないように。

## 拡張ポイント

将来的に増やすべき API:

- `ctx:include_graph()` — include グラフ (cross-file ルール用)
- `ctx:call_graph()`    — 関数呼び出しグラフ (例: 「main からしか呼ばない」)
- `ctx:lifetime_events()` — new/delete の対応追跡
- ノードの親参照 (`node:parent()`) — 現在は子方向のみ

これらは `cclint-ast` に解析パスを足し、`cclint-script` で `ctx` テーブルに
バインドする形で追加する。

## 注意

- `clang` crate は `Clang::new()` をプロセスで一回しか呼べない。
  `Session` で 1 回作って使い回す。
- 並列化は将来の課題。現状は逐次処理。
