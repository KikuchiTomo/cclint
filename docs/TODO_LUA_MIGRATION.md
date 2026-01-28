# Lua完全移行 TODOリスト

**最終更新**: 2026-01-28
**目的**: 組み込みルールを削除し、全てのルール定義をLuaスクリプトに移行する

## プロジェクトの新しいコンセプト

cclintは「Luaで独自にルールを定義できる」ことが最大の特徴です。
- 組み込みルールは一切提供しない（ゼロから自由に定義）
- Luaスクリプトで簡単にカスタムルールを作成できる
- C++のあらゆる要素（クラス、関数、enum、if文、インデント等）にアクセス可能
- メソッドチェーンで直感的にコードを探索・検証できる

## 凡例
- [ ] 未着手
- [>] 作業中
- [x] 完了
- [!] ブロック中
- [~] 保留

---

## Phase 1: 現状調査と設計 🔍

### 1.1 現在の組み込みルールの機能確認
- [ ] max_line_length.cpp の機能を確認
- [ ] header_guard.cpp の機能を確認
- [ ] function_complexity.cpp の機能を確認
- [ ] naming_convention.cpp の機能を確認
- [ ] 各ルールの設定パラメータを整理
- [ ] 既存のLuaルールとの対応関係を確認

### 1.2 Lua API拡張設計
以下の新しいAPIを設計：

#### ファイルレベルAPI
- [ ] `get_files()` - プロジェクト内の全ファイル取得
  - [ ] フィルタリング機能設計 (`.filter(pattern)`)
  - [ ] ファイル情報取得 (`.path()`, `.name()`, `.extension()`)
  - [ ] ファイルからコンテンツへ (`.lines()`, `.content()`)

#### 構造レベルAPI
- [ ] `get_functions()` - 全関数の取得
  - [ ] グローバル関数とメソッドの区別
  - [ ] パラメータ情報の取得
  - [ ] 戻り値型の取得

- [ ] `get_mains()` - main関数の特定
  - [ ] main関数から呼ばれる関数のチェック
  - [ ] main関数からしか呼べない関数の制限

- [ ] `get_enums()` - enum/enum classの取得
  - [ ] enum値の取得
  - [ ] enum classと通常のenumの区別

#### 制御構造API
- [ ] `get_ifs()` - if文の取得
  - [ ] ブレース強制チェック
  - [ ] else if/else の検出

- [ ] `get_switches()` - switch文の取得
- [ ] `get_loops()` - for/while文の取得
- [ ] `get_trys()` - try-catch文の取得

#### マクロAPI
- [ ] `get_macros()` - マクロ定義の取得
  - [ ] マクロ関数の検出
  - [ ] マクロ引数の取得

#### 関係性API
- [ ] `.callers()` - この関数を呼び出している関数一覧
- [ ] `.callees()` - この関数が呼び出している関数一覧
- [ ] `.called_from(function_name)` - 特定の関数から呼ばれているかチェック
- [ ] `.only_called_from(function_name)` - 特定の関数からのみ呼ばれることを強制

#### ファイル区分API
- [ ] `.header()` - ヘッダーファイルのみにフィルタ
- [ ] `.implementation()` - 実装ファイルのみにフィルタ
- [ ] `.is_header()` / `.is_implementation()` - 判定関数

#### インデント・スタイルAPI
- [ ] `.indent()` - インデント情報の取得
  - [ ] `.count()` - インデント数
  - [ ] `.type()` - スペースかタブか
  - [ ] `.width()` - インデント幅
- [ ] `.get_line_info(line_num)` - 行の詳細情報
  - [ ] 先頭の空白文字数
  - [ ] 実際のインデントレベル

#### メソッドチェーン機能
- [ ] `.each(callback)` - 各要素に対してコールバック実行
- [ ] `.filter(predicate)` - 条件でフィルタリング
- [ ] `.map(transform)` - 変換
- [ ] `.find(predicate)` - 最初にマッチする要素を検索
- [ ] `.count()` - 要素数のカウント

### 1.3 ASTノード拡張設計
- [ ] IfStatementNode の追加
- [ ] SwitchStatementNode の追加
- [ ] LoopStatementNode の追加 (for/while/do-while)
- [ ] TryStatementNode の追加
- [ ] MacroNode の追加
- [ ] CallExpressionNode の拡張（呼び出し関係の追跡）
- [ ] インデント情報をSourcePositionに追加

### 1.4 コールグラフ構築設計
- [ ] CallGraphBuilder クラス設計
  - [ ] 関数呼び出しの検出
  - [ ] 呼び出し元・呼び出し先の記録
  - [ ] グラフデータ構造の設計

---

## Phase 2: ASTパーサー拡張実装 🔧

### 2.1 SimpleParserの拡張
- [ ] マクロ定義のパース
  - [ ] `#define` の検出
  - [ ] マクロ関数の引数抽出
  - [ ] マクロ展開は行わない（定義のみ記録）

- [ ] 制御構造のパース
  - [ ] if文のパース
  - [ ] switch文のパース
  - [ ] for/while/do-while文のパース
  - [ ] try-catch文のパース
  - [ ] ブレースの有無を記録

- [ ] インデント情報の記録
  - [ ] 各行の先頭空白文字を記録
  - [ ] タブ/スペースの区別
  - [ ] インデントレベルの計算

### 2.2 コールグラフ構築
- [ ] CallGraphBuilder クラス実装
  - [ ] engine/call_graph_builder.hpp
  - [ ] engine/call_graph_builder.cpp
  - [ ] 関数呼び出しの検出ロジック
  - [ ] 呼び出し関係の記録

- [ ] ASTNode への関係情報追加
  - [ ] callers フィールド
  - [ ] callees フィールド

### 2.3 ASTノードの追加実装
- [ ] ast.hpp への新しいノード追加
  - [ ] IfStatementNode
  - [ ] SwitchStatementNode
  - [ ] LoopStatementNode
  - [ ] TryStatementNode
  - [ ] MacroNode
  - [ ] CallExpressionNode

- [ ] SourcePositionの拡張
  - [ ] indent_level フィールド追加
  - [ ] indent_type (space/tab) フィールド追加

---

## Phase 3: Lua API実装 💻

### 3.1 基本取得API
- [ ] `cclint.get_files()` の実装
  - [ ] lua_bridge.cpp に lua_get_files 追加
  - [ ] ファイル一覧をLuaテーブルとして返す

- [ ] `cclint.get_functions()` の実装
  - [ ] lua_get_functions 追加
  - [ ] 全関数（グローバル+メソッド）を返す

- [ ] `cclint.get_mains()` の実装
  - [ ] lua_get_mains 追加
  - [ ] main関数のみを返す

- [ ] `cclint.get_enums()` の実装
  - [ ] lua_get_enums 追加
  - [ ] enum/enum classを返す

- [ ] `cclint.get_ifs()` の実装
  - [ ] lua_get_ifs 追加
  - [ ] if文のノードを返す

- [ ] `cclint.get_switches()` の実装
- [ ] `cclint.get_loops()` の実装
- [ ] `cclint.get_macros()` の実装

### 3.2 関係性API
- [ ] `node.callers()` の実装
  - [ ] lua_get_callers 追加
  - [ ] 呼び出し元関数一覧を返す

- [ ] `node.callees()` の実装
  - [ ] lua_get_callees 追加
  - [ ] 呼び出し先関数一覧を返す

- [ ] `node.called_from(name)` の実装
- [ ] `node.only_called_from(name)` の実装

### 3.3 ファイル区分API
- [ ] ファイル情報の拡張
  - [ ] is_header フィールド追加
  - [ ] is_implementation フィールド追加

- [ ] `.header()` / `.implementation()` フィルタ実装

### 3.4 インデント・スタイルAPI
- [ ] `cclint.get_line_info(file, line)` の実装
  - [ ] lua_get_line_info 追加
  - [ ] インデント情報を含む行情報を返す

- [ ] `line.indent()` の実装
  - [ ] インデント数を返す
  - [ ] インデントタイプを返す

### 3.5 メソッドチェーン機能
Luaテーブルにメソッドを追加する形で実装：

- [ ] テーブルにメタテーブルを設定
  - [ ] __index メタメソッド
  - [ ] 各種メソッドの実装

- [ ] `.each(callback)` の実装
- [ ] `.filter(predicate)` の実装
- [ ] `.map(transform)` の実装
- [ ] `.find(predicate)` の実装
- [ ] `.count()` の実装

### 3.6 ナビゲーション機能
- [ ] `file.classes()` - ファイル内のクラス取得
- [ ] `file.functions()` - ファイル内の関数取得
- [ ] `class.methods()` - クラスのメソッド取得（既存機能の改善）

---

## Phase 4: 組み込みルール削除とLua版作成 🗑️

### 4.1 組み込みルールのLua版作成
既存の組み込みルールをLuaスクリプトで再実装：

- [ ] max_line_length のLua版
  - [ ] scripts/rules/style/max_line_length.lua （既存確認）
  - [ ] 機能の同等性確認

- [ ] header_guard のLua版
  - [ ] scripts/rules/structure/header_guard.lua （既存確認）
  - [ ] 機能の同等性確認

- [ ] function_complexity のLua版
  - [ ] 新しいAPI使用版の作成
  - [ ] サイクロマティック複雑度計算の実装

- [ ] naming_convention のLua版
  - [ ] scripts/rules/naming/naming_convention.lua の作成
  - [ ] アクセス指定子別の命名規則チェック

### 4.2 組み込みルールの削除
- [ ] src/rules/builtin/max_line_length.cpp 削除
- [ ] src/rules/builtin/max_line_length.hpp 削除
- [ ] src/rules/builtin/header_guard.cpp 削除
- [ ] src/rules/builtin/header_guard.hpp 削除
- [ ] src/rules/builtin/function_complexity.cpp 削除
- [ ] src/rules/builtin/function_complexity.hpp 削除
- [ ] src/rules/builtin/naming_convention.cpp 削除
- [ ] src/rules/builtin/naming_convention.hpp 削除
- [ ] src/rules/builtin/ ディレクトリ削除

### 4.3 ビルドシステムの更新
- [ ] src/CMakeLists.txt から組み込みルールの削除
- [ ] src/rules/CMakeLists.txt の更新（もし存在すれば）
- [ ] ビルドが正常に通ることを確認

### 4.4 ルールシステムの調整
- [ ] RuleRegistry から組み込みルール登録コードの削除
- [ ] main.cpp から組み込みルール初期化コードの削除
- [ ] ルール実行ロジックの確認（Luaルールのみ）

---

## Phase 5: YAML設定形式の変更 📝

### 5.1 設定ファイル形式の更新
現在の設定から組み込みルール参照を削除：

- [ ] .cclint.example.yaml の更新
  - [ ] 組み込みルール名の削除
  - [ ] Luaスクリプトパスのみの設定に変更
  - [ ] 新しいAPI使用例の追加

- [ ] 設定スキーマの更新
  - [ ] yaml_config.cpp の更新
  - [ ] rules セクションの扱いを変更（Luaスクリプトパスのみ）

### 5.2 サンプル設定の作成
- [ ] examples/configs/ 内の設定ファイル更新
  - [ ] 全てLuaスクリプトベースに変更
  - [ ] 新しいAPI使用例を含む設定

---

## Phase 6: ドキュメント更新 📚

### 6.1 README.md の更新
- [ ] プロジェクト概要の書き直し
  - [ ] 「組み込みルールなし、Lua完全カスタマイズ」を強調
  - [ ] 新しいAPIの簡単な紹介

- [ ] Quick Start セクションの更新
  - [ ] Luaスクリプト例の更新
  - [ ] 新しいメソッドチェーンの例

- [ ] Configuration セクションの更新
  - [ ] YAML設定から組み込みルール削除
  - [ ] Luaスクリプトのみの設定例

### 6.2 Lua API Reference の更新
- [ ] docs/lua_api.md の大幅な更新
  - [ ] 新しいAPI全てのドキュメント
  - [ ] メソッドチェーン例の追加
  - [ ] 実践的なルール例の追加

#### 追加するAPIセクション
- [ ] ファイルレベルAPI (`get_files()`)
- [ ] 関数レベルAPI (`get_functions()`, `get_mains()`)
- [ ] 制御構造API (`get_ifs()`, `get_switches()`, `get_loops()`)
- [ ] enum API (`get_enums()`)
- [ ] マクロAPI (`get_macros()`)
- [ ] 関係性API (`.callers()`, `.callees()`)
- [ ] ファイル区分API (`.header()`, `.implementation()`)
- [ ] インデントAPI (`get_line_info()`, `.indent()`)
- [ ] メソッドチェーンAPI (`.each()`, `.filter()`, `.map()`)

#### 実践的な例の追加
- [ ] 「main関数からのみ呼べる関数」ルール例
- [ ] 「特定の関数を呼んではいけない」ルール例
- [ ] 「enum classの使用を強制」ルール例
- [ ] 「if文にブレース必須」ルール例
- [ ] 「インデント幅チェック」ルール例
- [ ] 「ファイル名とクラス名の一致」ルール例

### 6.3 設計書の更新
- [ ] docs/design.md の更新
  - [ ] ルールシステムのアーキテクチャ変更を反映
  - [ ] 組み込みルールモジュールの削除
  - [ ] Lua APIの拡張を反映

- [ ] docs/detailed_design.md の更新
  - [ ] Lua Bridgeの詳細設計更新
  - [ ] 新しいASTノードの設計追加
  - [ ] コールグラフの設計追加

- [ ] docs/requirements.md の更新
  - [ ] 機能要件の変更
  - [ ] 非機能要件の追加（Lua API性能等）

### 6.4 サンプル・例の作成
- [ ] examples/rules/ の更新
  - [ ] 新しいAPIを使ったルール例
  - [ ] 複雑なルールの例（メソッドチェーン使用）
  - [ ] コメント付きのわかりやすい例

- [ ] scripts/rules/ の整理
  - [ ] 全てのルールが新しいAPIに対応
  - [ ] 各ルールに詳細なコメント追加

### 6.5 チュートリアルの作成
- [ ] docs/tutorial.md の作成
  - [ ] 「初めてのLuaルール作成」
  - [ ] 「メソッドチェーンの使い方」
  - [ ] 「複雑なルールの作成方法」
  - [ ] 「パフォーマンスを意識したルール作成」

---

## Phase 7: テスト 🧪

### 7.1 Lua APIのテスト
- [ ] tests/lua/ ディレクトリ作成
- [ ] 各API関数の単体テスト
  - [ ] get_files() テスト
  - [ ] get_functions() テスト
  - [ ] get_mains() テスト
  - [ ] get_enums() テスト
  - [ ] get_ifs() テスト
  - [ ] callers/callees テスト
  - [ ] インデント情報テスト
  - [ ] メソッドチェーンテスト

### 7.2 Luaルールのテスト
- [ ] 各Luaルールの動作確認
  - [ ] サンプルC++コードでテスト
  - [ ] 期待される診断が出力されることを確認

### 7.3 統合テスト
- [ ] エンドツーエンドテスト
  - [ ] 実際のプロジェクトでcclint実行
  - [ ] 複数のLuaルール同時実行
  - [ ] パフォーマンス測定

### 7.4 回帰テスト
- [ ] 既存機能が壊れていないことを確認
  - [ ] 従来のLua APIが動作すること
  - [ ] 設定ファイル読み込みが正常
  - [ ] 出力フォーマットが正常

---

## Phase 8: マイグレーションガイド作成 📖

### 8.1 ユーザー向けマイグレーションガイド
- [ ] docs/migration_guide.md の作成
  - [ ] 旧版からの移行手順
  - [ ] 組み込みルールからLuaルールへの変換方法
  - [ ] 設定ファイルの変更方法
  - [ ] よくある質問

### 8.2 破壊的変更のリスト
- [ ] CHANGELOG.md に記載
  - [ ] 組み込みルール削除
  - [ ] 設定ファイル形式の変更
  - [ ] 新しいLua API

---

## Phase 9: パフォーマンス最適化 ⚡

### 9.1 Lua API呼び出しの最適化
- [ ] 頻繁に呼ばれるAPIの最適化
- [ ] キャッシュ機構の実装
- [ ] 不要なコピーの削減

### 9.2 コールグラフ構築の最適化
- [ ] 遅延評価の実装
- [ ] メモリ使用量の削減

### 9.3 ベンチマーク
- [ ] 大規模プロジェクトでのベンチマーク
- [ ] 旧版との性能比較

---

## 優先順位

### 最優先（MVP）
1. Phase 2.1: SimpleParserの拡張（制御構造、インデント）
2. Phase 3.1: 基本取得API
3. Phase 4: 組み込みルール削除とLua版作成
4. Phase 6.1, 6.2: README.md と Lua API Reference の更新

### 高優先
5. Phase 2.2: コールグラフ構築
6. Phase 3.2: 関係性API
7. Phase 3.5: メソッドチェーン機能

### 中優先
8. Phase 3.4: インデント・スタイルAPI
9. Phase 6.3, 6.4: 設計書とサンプルの更新
10. Phase 7: テスト

### 低優先
11. Phase 9: パフォーマンス最適化
12. Phase 6.5: チュートリアル
13. Phase 8: マイグレーションガイド

---

## 進捗管理

### 完了基準
各フェーズの完了基準：
- [ ] 実装が完了している
- [ ] ビルドが通る
- [ ] 基本的なテストが通る
- [ ] ドキュメントが更新されている
- [ ] コミットされている

### マイルストーン
- **Milestone L1** (Lua Migration Phase 1): Phase 1-3完了
- **Milestone L2** (Lua Migration Phase 2): Phase 4-5完了
- **Milestone L3** (Lua Migration Phase 3): Phase 6-7完了
- **Milestone L4** (Lua Migration Phase 4): Phase 8-9完了

---

## 注意事項

### 破壊的変更
この移行は破壊的変更を含みます：
- 既存の組み込みルールを使用している設定ファイルは動作しなくなる
- ユーザーはLuaスクリプトへの移行が必要
- メジャーバージョンアップ（v1.0 → v2.0）として扱う

### 後方互換性
- 既存のLua APIは維持する
- 新しいAPIは追加のみ（既存APIの削除はしない）
- 段階的な移行をサポート

### コミュニケーション
- 変更内容を明確にユーザーに伝える
- マイグレーションガイドを充実させる
- サンプルコードを豊富に用意する

---

**このTODOリストは常に最新の状態を保つこと！**
作業開始前に必ず確認し、完了後に更新してください。
