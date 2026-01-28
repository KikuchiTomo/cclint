# Lua完全移行の影響範囲分析

**作成日**: 2026-01-28
**目的**: 組み込みルール削除とLua完全移行による影響を受けるファイル・コンポーネントを明確化

## 概要

この移行により、cclintは「組み込みルールなし、Lua完全カスタマイズ」というコンセプトに生まれ変わります。

### 主要な変更点
1. **組み込みルール（builtin rules）の完全削除**
2. **Lua APIの大幅な拡張**（メソッドチェーン、より詳細なAST操作）
3. **YAML設定形式の変更**（組み込みルール参照の削除）
4. **ドキュメント全面改訂**

---

## 影響を受けるコンポーネント

### 🔴 削除されるコンポーネント

#### src/rules/builtin/
- **max_line_length.cpp/.hpp** - 最大行長チェック
- **header_guard.cpp/.hpp** - ヘッダーガードチェック
- **function_complexity.cpp/.hpp** - 関数複雑度チェック
- **naming_convention.cpp/.hpp** - 命名規則チェック
- **builtin/ ディレクトリ全体**

理由: 全てLuaスクリプトで再実装するため

---

### 🟡 大きく変更されるコンポーネント

#### src/parser/
既存のパーサーを拡張して、より詳細な構造を解析：

- **ast.hpp** - ASTノード定義
  - 追加: IfStatementNode, SwitchStatementNode, LoopStatementNode
  - 追加: TryStatementNode, MacroNode, CallExpressionNode
  - 変更: SourcePosition にインデント情報追加
  - 変更: FunctionNode に callers/callees 追加

- **simple_parser.cpp** - 簡易パーサー実装
  - 追加: マクロ定義のパース
  - 追加: 制御構造（if, switch, for, while, try）のパース
  - 追加: インデント情報の記録
  - 追加: 関数呼び出しの検出

- **lexer.cpp** - 字句解析器
  - 改善: マクロトークンの処理
  - 改善: インデント情報の保持

#### src/lua/
Lua APIを大幅に拡張：

- **lua_bridge.hpp/.cpp** - C++ ⇔ Lua ブリッジ
  - 追加: lua_get_files() - ファイル一覧取得
  - 追加: lua_get_functions() - 関数一覧取得
  - 追加: lua_get_mains() - main関数取得
  - 追加: lua_get_enums() - enum取得
  - 追加: lua_get_ifs() - if文取得
  - 追加: lua_get_switches() - switch文取得
  - 追加: lua_get_loops() - ループ取得
  - 追加: lua_get_macros() - マクロ取得
  - 追加: lua_get_callers() - 呼び出し元取得
  - 追加: lua_get_callees() - 呼び出し先取得
  - 追加: lua_get_line_info() - 行情報取得
  - 追加: メソッドチェーン用のメタテーブル機能

#### src/engine/
コールグラフ構築機能の追加：

- **call_graph_builder.hpp/.cpp** - 新規作成
  - 関数呼び出し関係の解析
  - 呼び出しグラフの構築
  - callers/callees の記録

- **analysis_engine.cpp** - 解析エンジン
  - 変更: コールグラフ構築を統合
  - 変更: 拡張されたAST情報の処理

#### src/rules/
ルールシステムの簡素化：

- **rule_registry.cpp** - ルールレジストリ
  - 削除: 組み込みルールの登録コード
  - 簡素化: Luaルールのみの管理

- **rule_executor.cpp** - ルール実行器
  - 変更: 組み込みルール実行の削除
  - 確認: Luaルールのみの実行

#### src/config/
設定ファイル形式の変更：

- **yaml_config.cpp** - YAML設定パーサー
  - 変更: 組み込みルール参照の削除
  - 変更: Luaスクリプトパスのみの処理
  - 簡素化: ルール設定セクションの構造

---

### 🟢 新規作成されるコンポーネント

#### src/engine/
- **call_graph_builder.hpp** - コールグラフビルダー（ヘッダー）
- **call_graph_builder.cpp** - コールグラフビルダー（実装）

#### scripts/rules/
組み込みルールのLua版：

- **naming/naming_convention.lua** - 命名規則チェック（Lua版）
- **readability/function_complexity.lua** - 関数複雑度チェック（Lua版）
  - 注: 既存の function_cognitive_complexity.lua を拡張

---

### 📝 影響を受けるドキュメント

#### 大幅な更新が必要
- **README.md** - プロジェクト概要の全面改訂
  - コンセプトの書き直し（「組み込みルールなし」を強調）
  - Quick Startの更新
  - Lua API例の追加
  - 設定例の変更

- **docs/lua_api.md** - Lua APIリファレンスの全面改訂
  - 新しいAPI全てのドキュメント（20以上のAPI追加）
  - メソッドチェーン例の追加
  - 実践的なルール例の大幅追加

- **docs/design.md** - 設計書の更新
  - ルールシステムのアーキテクチャ変更
  - コールグラフの設計追加
  - 拡張されたASTの設計

- **docs/detailed_design.md** - 詳細設計の更新
  - Lua Bridgeの詳細設計
  - 新しいASTノードの詳細
  - コールグラフビルダーの詳細

- **docs/requirements.md** - 要件定義の更新
  - 機能要件の変更
  - 非機能要件の追加

#### 中程度の更新が必要
- **docs/usage.md** - 使い方ガイド
  - Luaルール作成方法の更新
  - 設定ファイル例の変更

- **docs/FAQ.md** - FAQ
  - 新しいAPIに関する質問追加
  - 組み込みルール削除に関する質問

- **CONTRIBUTING.md** - 貢献ガイド
  - ルール開発ガイドラインの更新

#### 新規作成が必要
- **docs/tutorial.md** - チュートリアル
  - 初めてのLuaルール作成
  - メソッドチェーンの使い方
  - 複雑なルールの作成例

- **docs/migration_guide.md** - マイグレーションガイド
  - v1.x から v2.0 への移行手順
  - 組み込みルールからLuaへの変換例
  - 設定ファイルの変更方法

---

### ⚙️ 影響を受ける設定ファイル

#### .cclint.example.yaml
- **削除**: 組み込みルール参照
  - `naming_conventions`
  - `header_guards`
  - `max_line_length`
  - `function_complexity`

- **追加**: Luaスクリプトパスのみの設定
  - 各カテゴリのLuaルール例
  - 新しいAPI使用例

#### examples/configs/
- **全てのサンプル設定ファイル更新**
  - 組み込みルール参照の削除
  - Luaスクリプトベースに変更

---

### 🧪 影響を受けるテスト

#### tests/rules/builtin/
- **削除**: 組み込みルールのテストファイル全て
  - test_naming_convention.cpp
  - test_header_guard.cpp
  - test_max_line_length.cpp
  - test_function_complexity.cpp

#### tests/lua/
- **新規作成**: Lua APIテスト
  - test_get_files.cpp
  - test_get_functions.cpp
  - test_callers_callees.cpp
  - test_method_chaining.cpp
  - test_indent_api.cpp

#### tests/integration/
- **更新**: 統合テスト
  - Luaルールのみの実行テスト
  - 新しいAPIを使ったルールのテスト

---

### 🏗️ ビルドシステム

#### CMakeLists.txt
- **src/CMakeLists.txt**
  - 削除: builtin/ ディレクトリのビルド設定
  - 削除: 組み込みルールのソースファイル
  - 確認: Luaルールのインストール設定

#### scripts/rules/ の構成
- **変更なし**: 既存のLuaスクリプト
- **追加**: 組み込みルールから変換したLuaスクリプト
- **更新**: 新しいAPIを使ったスクリプト例

---

## ファイル変更サマリー

### 削除 (🔴)
```
src/rules/builtin/
├── max_line_length.cpp
├── max_line_length.hpp
├── header_guard.cpp
├── header_guard.hpp
├── function_complexity.cpp
├── function_complexity.hpp
├── naming_convention.cpp
└── naming_convention.hpp

tests/rules/builtin/
├── test_naming_convention.cpp
├── test_header_guard.cpp
├── test_max_line_length.cpp
└── test_function_complexity.cpp
```

### 新規作成 (🟢)
```
src/engine/
├── call_graph_builder.hpp
└── call_graph_builder.cpp

scripts/rules/naming/
└── naming_convention.lua

docs/
├── tutorial.md
├── migration_guide.md
└── MIGRATION_IMPACT.md (this file)

tests/lua/
├── test_get_files.cpp
├── test_get_functions.cpp
├── test_callers_callees.cpp
├── test_method_chaining.cpp
└── test_indent_api.cpp
```

### 大幅な変更 (🟡)
```
src/parser/
├── ast.hpp                      (ASTノード追加)
├── simple_parser.cpp            (制御構造、マクロ、インデント対応)
└── lexer.cpp                    (マクロ、インデント処理)

src/lua/
├── lua_bridge.hpp               (10以上の新API追加)
└── lua_bridge.cpp               (実装)

src/engine/
└── analysis_engine.cpp          (コールグラフ統合)

src/rules/
├── rule_registry.cpp            (組み込みルール削除)
└── rule_executor.cpp            (Luaのみ)

src/config/
└── yaml_config.cpp              (設定形式変更)

docs/
├── README.md                    (全面改訂)
├── lua_api.md                   (全面改訂)
├── design.md                    (更新)
├── detailed_design.md           (更新)
└── requirements.md              (更新)

.cclint.example.yaml             (全面改訂)
```

---

## 影響を受けるユーザー

### 既存ユーザー
- **組み込みルールを使用している場合**: Luaスクリプトへの移行が必要
- **設定ファイル**: 更新が必要
- **カスタムルール**: 既存のLua APIは維持されるため影響なし

### 新規ユーザー
- **メリット**: よりシンプルで一貫性のあるAPI
- **学習曲線**: Luaルール作成の学習が必要（組み込みルールは使えない）

---

## リスク評価

### 高リスク
- **後方互換性の破壊**: 既存の設定ファイルが動作しなくなる
- **Lua API実装の複雑さ**: メソッドチェーンやコールグラフの実装は複雑
- **パフォーマンス**: 新しいAPI呼び出しのオーバーヘッド

### 中リスク
- **ドキュメントの整合性**: 多数のドキュメント更新が必要
- **テストカバレッジ**: 新しいAPIの十分なテスト作成

### 低リスク
- **ビルドシステム**: 組み込みルール削除は単純な作業
- **既存のLua API**: 変更がないため影響なし

---

## マイグレーション戦略

### 段階的リリース
1. **v2.0-alpha**: Phase 1-3完了（新API追加、組み込みルール削除）
2. **v2.0-beta**: Phase 4-7完了（ドキュメント、テスト）
3. **v2.0-rc**: Phase 8-9完了（マイグレーションガイド、最適化）
4. **v2.0**: 正式リリース

### サポート期間
- **v1.x**: v2.0リリース後6ヶ月間サポート（セキュリティ修正のみ）
- **v2.0**: 長期サポート（LTS）

---

## 見積もり

### 工数見積もり（概算）
- **Phase 1**: 調査・設計 - 2-3日
- **Phase 2**: ASTパーサー拡張 - 5-7日
- **Phase 3**: Lua API実装 - 7-10日
- **Phase 4**: 組み込みルール削除 - 2-3日
- **Phase 5**: YAML設定変更 - 1-2日
- **Phase 6**: ドキュメント更新 - 5-7日
- **Phase 7**: テスト - 3-5日
- **Phase 8**: マイグレーションガイド - 2-3日
- **Phase 9**: パフォーマンス最適化 - 3-5日

**合計**: 30-45日（1-1.5ヶ月）

### 優先順位付き工数
- **MVP（最小動作版）**: 15-20日
- **完全版**: 30-45日

---

## 次のアクション

1. ✅ TODOリスト作成（完了）
2. ✅ 影響範囲分析（完了）
3. [ ] Phase 1開始: 現状調査と設計
4. [ ] プロトタイプの作成（新API 2-3個）
5. [ ] 実装計画の最終確認

---

**この影響分析は開発の指針として使用してください。**
実装中に新たな影響が判明した場合は、このドキュメントを更新してください。
