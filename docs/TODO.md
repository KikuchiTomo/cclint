# cclint TODO リスト

**最終更新**: 2026-01-23

このドキュメントはプロジェクトの進捗を管理するためのTODOリストです。
作業を開始する前に必ずこのリストを確認し、作業が完了したら更新してください。

## 凡例
- [ ] 未着手
- [>] 作業中
- [x] 完了
- [!] ブロック中（依存関係待ち）
- [~] 保留

---

## Milestone 0: プロジェクトセットアップ ✅ 完了

### ドキュメント作成
- [x] ディレクトリ構造の作成（docs/, src/）
- [x] 要件定義書の作成（docs/requirements.md）
- [x] 設計書の作成（docs/design.md）
- [x] 詳細設計書の作成（docs/detailed_design.md）
- [x] マイルストーンの作成（docs/milestones.md）
- [x] TODOリストの作成（docs/TODO.md）
- [x] README.mdの作成
- [x] CLAUDE.mdの作成
  - [x] TODOリスト運用方法の明記
  - [x] 開発ガイドラインの記述
  - [x] Claude Code利用方法の記述

### プロジェクト設定
- [x] .gitignoreの作成
- [x] .clang-formatの作成
- [x] .clang-tidyの作成
- [x] LICENSEファイルの追加（リモートに既存）

### ビルドシステム（Milestone 1に移動）
- [x] CMakeLists.txt（ルート）の作成
- [x] CMakeLists.txt（src/）の作成
- [ ] CMakeLists.txt（tests/）の作成
- [>] 依存関係の設定（外部ライブラリ統合は後続マイルストーン）
  - [!] LLVM/Clangの検出（Milestone 2）
  - [!] yaml-cppの検出（Milestone 2）
  - [!] LuaJIT 2.1の組み込み（Milestone 3）

### 環境確認（Milestone 1に移動）
- [ ] Ubuntu環境でのビルド確認
- [ ] macOS環境でのビルド確認
- [ ] 依存ライブラリのバージョン確認

---

## Milestone 1: 最小動作版（MVP） - 進捗: 80%

**完了済み**: CLI, Config, Compiler, Diagnostic, Output, Utils, Logger (7モジュール)
**実装済み**: 100個の標準Luaルールスクリプト
**ブロック中**: Parser (LLVM/Clang依存), Rule Engine (LuaJIT依存)
**統合待ち**: Main.cppへの各モジュール統合

### 1. CLI Module (src/cli/)

#### ArgumentParser
- [x] argument_parser.hpp の作成
- [x] argument_parser.cpp の実装
  - [x] コマンドライン引数の基本解析
  - [x] cclintオプションの抽出
  - [x] コンパイラコマンドの抽出
  - [x] `--config` オプション対応
  - [x] `--format` オプション対応
  - [x] `-v/--verbose` オプション対応
  - [x] `-q/--quiet` オプション対応
  - [x] `--help` オプション対応
  - [x] `--version` オプション対応
- [x] help_formatter.hpp の作成
- [x] help_formatter.cpp の実装
  - [x] ヘルプメッセージの生成
  - [x] 使用例の表示
  - [x] オプション一覧の表示
- [ ] 単体テスト（tests/cli/test_argument_parser.cpp）

### 2. Config Module (src/config/)

#### データ構造
- [x] config_types.hpp の作成
  - [x] Severity列挙型の定義
  - [x] RuleConfig構造体の定義
  - [x] Config構造体の定義

#### ConfigLoader
- [x] config_loader.hpp の作成
- [x] config_loader.cpp の実装
  - [x] 設定ファイルの探索
    - [x] カレントディレクトリ (.cclint.yaml)
    - [x] カレントディレクトリ (cclint.yaml)
    - [x] プロジェクトルート (cclint.yaml)
    - [x] ホームディレクトリ (~/.cclint/config.yaml)
  - [x] 設定ファイルの読み込み
  - [x] デフォルト値の適用
  - [x] 設定の検証

#### YAML Parser
- [x] yaml_config.hpp の作成
- [x] yaml_config.cpp の実装（スタブ実装、Milestone 2で完成予定）
  - [x] yaml-cppを使ったパース（構造のみ実装、yaml-cpp導入待ち）
  - [x] version フィールドの読み込み（実装準備完了）
  - [x] cpp_standard フィールドの読み込み（実装準備完了）
  - [x] rules フィールドの読み込み（実装準備完了）
  - [x] include_patterns フィールドの読み込み（実装準備完了）
  - [x] exclude_patterns フィールドの読み込み（実装準備完了）
  - [x] lua_scripts フィールドの読み込み（実装準備完了）
  - [x] output フィールドの読み込み（実装準備完了）
  - [x] パースエラーのハンドリング（実装準備完了）
- [ ] 単体テスト（tests/config/test_config_loader.cpp）

### 3. Compiler Module (src/compiler/)

#### CompilerWrapper
- [x] wrapper.hpp の作成
- [x] wrapper.cpp の実装
  - [x] コマンド実行機能
    - [x] popen を使った実行
    - [x] 標準出力のキャプチャ
    - [x] 標準エラー出力のキャプチャ
    - [x] 終了コードの取得
  - [x] ソースファイルの抽出
    - [x] .cpp, .cc, .cxx ファイルの検出
    - [x] .c ファイルの検出
    - [x] .h, .hpp ファイルの検出
  - [x] コンパイラフラグの抽出
    - [x] -I, -D, -std フラグの抽出
    - [x] その他のフラグの抽出
- [x] detector.hpp の作成
- [x] detector.cpp の実装
  - [x] gccの検出
  - [x] clangの検出
  - [x] コンパイラバージョンの取得
- [ ] 単体テスト（tests/compiler/test_wrapper.cpp）

### 4. Parser Module (src/parser/)

#### AST定義
- [ ] ast.hpp の作成
  - [ ] ASTノードの基底クラス定義

#### ClangParser
- [ ] clang_parser.hpp の作成
- [ ] clang_parser.cpp の実装
  - [ ] Clang libtooling の初期化
  - [ ] ASTUnitの生成
  - [ ] コンパイラフラグの変換
  - [ ] パースエラーのハンドリング
- [ ] ASTVisitor基底クラスの実装
  - [ ] VisitFunctionDecl
  - [ ] VisitVarDecl
  - [ ] VisitCXXRecordDecl
  - [ ] VisitNamespaceDecl
- [ ] 単体テスト（tests/parser/test_clang_parser.cpp）

### 5. Diagnostic Module (src/diagnostic/)

#### Diagnostic定義
- [x] diagnostic.hpp の作成
  - [x] Severity列挙型
  - [x] SourceLocation構造体
  - [x] SourceRange構造体
  - [x] FixItHint構造体
  - [x] Diagnostic構造体
  - [x] DiagnosticEngine（診断メッセージ管理）
- [x] diagnostic.cpp の実装
  - [x] 診断の追加
  - [x] 診断の文字列化
  - [x] エラー数/警告数の集計
- [ ] 単体テスト（tests/diagnostic/test_diagnostic.cpp）

### 6. Output Module (src/output/)

#### Formatter
- [x] formatter.hpp の作成（基底クラス）
- [x] formatter.cpp の実装
  - [x] 統計情報計算
  - [x] ヘッダー/フッター出力

#### TextFormatter
- [x] text_formatter.hpp の作成
- [x] text_formatter.cpp の実装
  - [x] 基本的なテキスト出力
  - [x] カラー出力（ANSIエスケープシーケンス）
  - [x] severity の表示
  - [x] ファイル名:行番号:列番号 の表示
  - [x] 統計サマリー

#### JsonFormatter
- [x] json_formatter.hpp の作成
- [x] json_formatter.cpp の実装
  - [x] JSON形式の出力
  - [x] JSONエスケープ処理
  - [x] 統計情報の出力

#### XmlFormatter
- [x] xml_formatter.hpp の作成
- [x] xml_formatter.cpp の実装
  - [x] XML形式の出力
  - [x] XMLエスケープ処理
  - [x] XMLヘッダー/フッター

#### FormatterFactory
- [x] formatter_factory.hpp の作成
- [x] formatter_factory.cpp の実装
  - [x] フォーマット名からフォーマッタを生成
  - [x] サポートフォーマットの検証

- [ ] 単体テスト（tests/output/test_formatter.cpp）

### 7. Utility Module (src/utils/)

#### ファイルユーティリティ
- [x] file_utils.hpp の作成
- [x] file_utils.cpp の実装
  - [x] ファイル読み込み
  - [x] ファイル書き込み
  - [x] ファイル存在チェック
  - [x] ディレクトリ操作
  - [x] パス操作（絶対/相対/正規化）
  - [x] グロブパターンマッチング（簡易版）

#### 文字列ユーティリティ
- [x] string_utils.hpp の作成
- [x] string_utils.cpp の実装
  - [x] 文字列分割
  - [x] 文字列結合
  - [x] trim
  - [x] 大文字/小文字変換
  - [x] 文字列置換
  - [x] 文字列検索

#### ロガー
- [x] logger.hpp の作成
- [x] logger.cpp の実装
  - [x] ログレベル設定
  - [x] ログ出力（stdout/stderr）
  - [x] デバッグログ

### 8. Main Entry Point

#### main.cpp
- [x] main.cpp の作成
  - [>] 基本的な実行フロー（部分実装）
    - [x] 引数解析
    - [!] 設定読み込み（ConfigLoader実装済、main.cpp統合待ち）
    - [!] ソースファイル抽出（CompilerWrapper実装済、main.cpp統合待ち）
    - [!] パース（AST構築）（LLVM/Clang依存、Milestone 2）
    - [!] コンパイラ実行（オプション）（CompilerWrapper実装済、main.cpp統合待ち）
    - [!] 結果出力（Formatter実装済、main.cpp統合待ち）
  - [x] エラーハンドリング
    - [x] try-catch
    - [x] 終了コードの決定
  - [x] --help, --version の処理

### 9. Build & Testing

#### ビルドシステム
- [x] CMakeLists.txtの完成（基本ビルドシステム）
- [x] ビルドスクリプトの作成（build.sh）
- [x] インストールターゲットの作成（バイナリとLuaスクリプト）

#### テスト
- [ ] Google Testの統合
- [x] テストディレクトリの作成（tests/）
  - [x] tests/unit/ (単体テスト)
  - [x] tests/integration/ (統合テスト)
  - [x] tests/samples/ (テスト用サンプルコード)
  - [x] tests/README.md (テストガイド)
  - [x] テスト用CMakeLists.txt
  - [x] サンプルコード (valid, invalid, style_issues, security_issues, performance_issues)
- [ ] 統合テスト（tests/integration/）
  - [ ] 基本的な実行テスト
  - [ ] エラーハンドリングのテスト

#### CI/CD
- [x] GitHub Actions設定（.github/workflows/ci.yml）
  - [x] Ubuntu環境でのビルド
  - [x] macOS環境でのビルド
  - [x] テスト実行
  - [x] コードカバレッジ

### 10. Lua Rule Scripts (scripts/rules/)

#### 標準ルールライブラリ
- [x] **100個の標準Luaルールスクリプト作成完了**
- [x] Naming ルール（9個）: class-name-camelcase, function-name-lowercase, constant-name-uppercase, enum-name-camelcase, namespace-name-lowercase, member-variable-prefix, typedef-suffix, global-variable-prefix, bool-variable-prefix, macro-name-uppercase, template-parameter-name
- [x] Style ルール（8個）: braces-on-new-line, indent-width, max-line-length, space-after-control-statement, consistent-pointer-declaration, consistent-bracing, namespace-closing-comment, function-definition-style, empty-line-before-block
- [x] Structure ルール（4個）: one-class-per-file, header-guard, include-order, forward-declaration-namespace
- [x] Spacing ルール（5個）: max-consecutive-empty-lines, trailing-whitespace, operator-spacing, no-tab-character, blank-line-after-declaration
- [x] Documentation ルール（4個）: require-function-comments, todo-comment, file-header, copyright-header
- [x] Modernize ルール（18個）: use-nullptr, use-auto, use-override, use-using, use-noexcept, use-nodiscard, use-emplace, use-equals-default, use-equals-delete, use-default-member-init, use-constexpr, raw-string-literal, use-final, use-designated-initializers, use-enum-class, use-transparent-comparators, use-lambda, avoid-bind, use-range-based-for, use-structured-bindings
- [x] Performance ルール（15個）: pass-by-const-reference, avoid-unnecessary-copy, move-const-arg, reserve-vector, static-const-string, unnecessary-value-param, inline-small-functions, make-shared-preference, loop-invariant-code, prefer-prefix-increment, string-concatenation, redundant-string-init, avoid-temporary-objects, prefer-algorithm, avoid-default-arguments-virtual
- [x] Readability ルール（19個）: max-function-length, no-magic-numbers, simplify-boolean-expr, switch-has-default, unused-parameter, else-after-return, avoid-nested-conditionals, consistent-declaration-parameter-name, identifier-naming, prefer-nullptr-comparison, function-cognitive-complexity, explicit-bool-conversion, avoid-c-cast, redundant-declaration, confusing-else, implicit-fallthrough, misleading-indentation, variables-one-per-line, comparison-order, multiline-comment-style
- [x] Security ルール（11個）: no-unsafe-functions, check-array-bounds, memset-zero-length, no-rand, no-system-call, hardcoded-credentials, integer-overflow, signed-unsigned-comparison, uninitialized-variable, buffer-overflow-risk, null-pointer-dereference, toctou-race
- [x] scripts/README.md 作成（ルール使用方法ドキュメント）
- [x] .cclint.example.yaml 更新（全100ルールの設定例）

### 11. Documentation

- [x] ビルド手順のドキュメント（docs/build.md）
- [x] 基本的な使い方（docs/usage.md）
- [x] トラブルシューティング（docs/troubleshooting.md）
- [x] FAQ（docs/FAQ.md）
- [x] 貢献ガイド（CONTRIBUTING.md）
- [>] 行動規範（CODE_OF_CONDUCT.md）
- [ ] 変更履歴（CHANGELOG.md）

---

## Milestone 2: ルールシステム基盤 ✅ 完了

### 1. Rules Module (src/rules/)

#### 基底クラス
- [x] rule_base.hpp の作成
  - [x] RuleBaseインターフェース
  - [x] 診断生成ヘルパー
- [x] rule_base.cpp の実装

#### RuleRegistry
- [x] rule_registry.hpp の作成
- [x] rule_registry.cpp の実装
  - [x] シングルトンパターン
  - [x] ルール登録
  - [x] ルール有効化/無効化
  - [x] ルール検索
  - [x] 全ルール名の取得
- [ ] 単体テスト（tests/rules/test_rule_registry.cpp）

#### RuleExecutor
- [x] rule_executor.hpp の作成
- [x] rule_executor.cpp の実装
  - [x] ルール実行ループ
  - [x] エラーハンドリング
  - [~] タイムアウト処理（将来実装）
- [ ] 単体テスト（tests/rules/test_rule_executor.cpp）

### 2. Builtin Rules (src/rules/builtin/)

#### 命名規則ルール
- [x] naming_convention.hpp の作成
- [x] naming_convention.cpp の実装
  - [x] 関数名チェック（snake_case）
  - [~] 変数名チェック（将来実装）
  - [x] クラス名チェック（PascalCase）
  - [~] 定数名チェック（将来実装）
- [ ] 単体テスト（tests/rules/builtin/test_naming_convention.cpp）

#### ヘッダーガードルール
- [x] header_guard.hpp の作成
- [x] header_guard.cpp の実装
  - [x] #ifndef/#define/#endifの検出
  - [x] #pragma onceの検出
  - [x] ガード名のチェック
- [ ] 単体テスト（tests/rules/builtin/test_header_guard.cpp）

#### 最大行長ルール
- [x] max_line_length.hpp の作成
- [x] max_line_length.cpp の実装
  - [x] 行長のチェック
  - [x] 設定可能な最大長
- [ ] 単体テスト（tests/rules/builtin/test_max_line_length.cpp）

### 3. Engine Module (src/engine/)

#### AnalysisEngine
- [x] analysis_engine.hpp の作成
- [x] analysis_engine.cpp の実装
  - [x] ファイルごとの解析
  - [x] ルール実行の統合
  - [x] ファイルフィルタリング（include/exclude）
  - [x] エラーハンドリング
- [ ] 単体テスト（tests/engine/test_analysis_engine.cpp）

### 4. Config Enhancement

- [x] ルール設定のYAML定義サポート
- [x] ルールパラメータの設定
- [x] severity設定（error/warning/info）
- [x] ルールごとの有効化/無効化

### 5. Testing & Integration

- [ ] 統合テスト（tests/integration/）
  - [x] 複数ルールの実行（動作確認済み）
  - [ ] 設定ファイルとの統合
  - [ ] エンドツーエンドテスト
- [x] サンプルC++ファイルの作成（tests/samples/）

---

## Milestone 3: Lua統合 ✅ 完了（LuaJIT利用可能時）

### 1. Lua Module (src/lua/)

#### LuaEngine
- [x] lua_engine.hpp の作成
- [x] lua_engine.cpp の実装（条件付きコンパイル）
  - [x] Lua VMの初期化（LuaJIT利用可能時）
  - [x] スクリプトのロード
  - [x] エラーハンドリング
  - [x] リソース管理（デストラクタ）
  - [x] スタブ実装（LuaJIT不在時）
- [ ] 単体テスト（tests/lua/test_lua_engine.cpp）

#### LuaRule
- [x] lua_rule.hpp の作成
- [x] lua_rule.cpp の実装
  - [x] RuleBase継承
  - [x] スクリプトロード
  - [x] check_file実装
  - [~] check_ast実装（LLVM/Clang依存のため保留）

#### Lua Bridge
- [x] lua_bridge.hpp の作成
- [x] lua_bridge.cpp の実装
  - [x] C++からLua関数の呼び出し
  - [x] LuaからC++関数の呼び出し
  - [x] データ型変換（C++ ⇔ Lua）
  - [~] ASTノードのLua公開（LLVM/Clang依存のため保留）

### 2. Lua API Implementation

#### 診断報告API
- [x] report_error の実装
- [x] report_warning の実装
- [x] report_info の実装

#### ASTアクセスAPI
- [~] get_node_type の実装（LLVM/Clang依存のため保留）
- [~] get_node_name の実装（LLVM/Clang依存のため保留）
- [~] get_node_location の実装（LLVM/Clang依存のため保留）
- [~] get_children の実装（LLVM/Clang依存のため保留）
- [~] get_parent の実装（LLVM/Clang依存のため保留）

#### ユーティリティAPI
- [x] match_pattern の実装
- [x] get_file_content の実装
- [~] get_source_range の実装（LLVM/Clang依存のため保留）

### 3. Lua Sandbox

#### セキュリティ制限
- [x] ファイルI/O制限の実装
  - [x] io.open の制限
  - [x] io.popen の無効化
- [x] ネットワークアクセス禁止
  - [x] socket ライブラリの無効化
- [x] システムコマンド実行禁止
  - [x] os.execute の無効化
  - [x] os.exit の無効化

#### リソース制限
- [~] メモリ使用量制限（v2.0以降で実装予定）
- [~] 実行時間制限（タイムアウト）（v2.0以降で実装予定）
- [~] スタック深度制限（v2.0以降で実装予定）

### 4. Lua Rule System

#### ルール登録
- [x] Luaルールの登録機能
- [x] ルール関数の呼び出し（check_file）
- [x] ルール実行エラーのハンドリング

#### ルール設定
- [x] パラメータの受け渡し
- [x] メタデータ読み込み（description, category）
- [x] ファイル内容のLuaへの渡し

### 5. Sample Lua Rules

- [x] サイクロマティック複雑度チェック（examples/rules/complexity.lua）
- [x] TODOコメント検出（examples/rules/todo_detector.lua）
- [x] 使用例ルール（examples/rules/example_rule.lua）

### 6. Documentation

- [x] サンプルルールREADME（examples/rules/README.md）
- [~] Lua API リファレンス（docs/lua_api.md）（後で作成）
- [~] Luaルール作成ガイド（examples/rules/README.mdに統合済み）

### 7. Testing

- [ ] Lua APIのテスト
- [ ] Luaサンドボックスのテスト
- [ ] サンプルルールのテスト
- [ ] エラーハンドリングのテスト

---

## Milestone 4: パフォーマンス最適化

### 1. Parallel Processing

#### マルチスレッド解析
- [ ] スレッドプール実装
- [ ] ファイル並列処理
- [ ] CPU コア数自動検出
- [ ] スレッド数の設定オプション

#### スレッドセーフ対策
- [ ] DiagnosticReporterのロック
- [ ] RuleRegistryのロック
- [ ] 共有リソースの保護

### 2. Caching System

#### パース結果キャッシュ
- [ ] ファイルハッシュの計算（SHA256）
- [ ] キャッシュの保存
- [ ] キャッシュの読み込み
- [ ] キャッシュの無効化

#### キャッシュストレージ
- [ ] ディスクベースキャッシュ
- [ ] キャッシュディレクトリ管理（.cclint_cache/）
- [ ] キャッシュのクリーンアップ

### 3. Incremental Analysis

- [ ] 変更ファイルの検出
  - [ ] git diffとの統合
  - [ ] タイムスタンプチェック
- [ ] 部分解析
- [ ] 依存関係の追跡

### 4. Performance Monitoring

- [ ] 解析時間の計測
- [ ] メモリ使用量の計測
- [ ] ボトルネックの分析
- [ ] プロファイリングモード（--profile）

### 5. Optimization

- [ ] ASTのメモリ最適化
- [ ] 不要なコピーの削減（move semantics）
- [ ] 早期終了の実装（--max-errors）
- [ ] ルール実行の最適化

### 6. Testing

- [ ] パフォーマンステスト
- [ ] 大規模プロジェクトでのテスト
- [ ] ベンチマークスイートの作成
- [ ] メモリリーク検査（valgrind）

---

## Milestone 5: 出力フォーマットとツール統合 ✅ 完了

### 1. JSON Formatter

- [x] json_formatter.hpp の作成
- [x] json_formatter.cpp の実装
  - [x] 診断のJSON変換
  - [x] JSON出力
  - [x] 統計情報
- [ ] 単体テスト

### 2. XML Formatter

- [x] xml_formatter.hpp の作成
- [x] xml_formatter.cpp の実装
  - [x] 診断のXML変換
  - [x] checkstyle形式対応
  - [x] XMLエスケープ処理
- [ ] 単体テスト

### 3. Enhanced Text Formatter

- [x] カラー出力（ANSIエスケープシーケンス）
- [x] ソースコンテキスト表示
- [x] 統計情報サマリー
- [~] 修正提案の表示（v2.0以降で実装予定）

### 4. CI/CD Integration

- [x] GitHub Actions統合（.github/workflows/ci.yml）
- [x] GitLab CI統合例（examples/ci/gitlab-ci.yml）
- [x] Jenkins統合例（examples/ci/Jenkinsfile）
- [x] 終了コードの制御

### 5. IDE Integration

#### VSCode
- [x] tasks.json サンプル（examples/ide/vscode/tasks.json）
- [x] settings.json サンプル（examples/ide/vscode/settings.json）
- [x] problem matcher定義

#### Vim/Neovim
- [x] ALE統合サンプル（examples/ide/vim/cclint.vim）
- [x] quickfix対応
- [x] 統合ガイド（examples/ide/vim/README.md）

### 6. Documentation

- [x] CI/CD統合ガイド（docs/ci_integration.md）
- [x] IDE統合ガイド（docs/ide_integration.md）
- [x] 出力フォーマットリファレンス（docs/output_formats.md）

### 7. Testing

- [ ] 各フォーマッターのテスト
- [ ] CI環境でのテスト
- [ ] 統合テスト

---

## Milestone 6: リリース準備

### 1. Documentation

- [ ] README.md完成版
- [ ] INSTALL.md の作成
- [ ] USER_GUIDE.md の作成
- [ ] API_REFERENCE.md の作成
- [ ] TROUBLESHOOTING.md の作成
- [ ] FAQ.md の作成
- [ ] CHANGELOG.md の作成
- [ ] CONTRIBUTING.md の作成
- [ ] CODE_OF_CONDUCT.md の作成

### 2. Examples & Samples

- [ ] サンプルプロジェクトの作成（examples/sample_project/）
- [ ] YAML設定サンプル集（examples/configs/）
- [ ] Luaルールサンプル集（examples/rules/）
- [ ] チュートリアルの作成（docs/tutorial.md）

### 3. Packaging

#### Debian Package
- [ ] debian/ ディレクトリの作成
- [ ] control ファイルの作成
- [ ] パッケージビルドスクリプト
- [ ] 依存関係の定義
- [ ] テスト

#### Homebrew Formula
- [ ] Formula作成（cclint.rb）
- [ ] Homebrewリポジトリへの登録準備
- [ ] テスト

#### Source Tarball
- [ ] リリースアーカイブ作成スクリプト
- [ ] バージョン管理

### 4. Installation

- [ ] install.sh スクリプトの作成
- [ ] uninstall.sh スクリプトの作成
- [ ] 依存関係の自動インストール
- [ ] インストールテスト

### 5. Quality Assurance

- [ ] すべてのテストの実行
- [ ] コードカバレッジ確認（目標80%以上）
- [ ] メモリリーク検査（valgrind）
- [ ] 静的解析（clang-tidy）
- [ ] 静的解析（cppcheck）
- [ ] ドキュメントレビュー
- [ ] コードレビュー

### 6. Release Process

- [ ] バージョン番号の設定（1.0.0）
- [ ] リリースノートの作成
- [ ] GitHubリリースの作成
- [ ] Git タグの作成（v1.0.0）
- [ ] リリースアナウンス

### 7. Website & Community

- [ ] GitHub Pagesの設定
- [ ] プロジェクトWebサイトの作成
- [ ] ロゴの作成
- [ ] Issue テンプレートの作成
- [ ] Pull Request テンプレートの作成

---

## 将来的な拡張（v2.0以降）

### 自動修正機能
- [ ] fixitヒントの実装
- [ ] 自動修正モード（--fix）
- [ ] 修正プレビュー

### データフロー解析
- [ ] より詳細なデータフロー解析
- [ ] 未初期化変数の検出
- [ ] nullポインタチェック

### プラグインシステム
- [ ] 共有ライブラリとしてのルール
- [ ] プラグインAPI
- [ ] 動的ロード

### Windows対応
- [ ] Windows環境でのビルド
- [ ] MSVCサポート
- [ ] パッケージング（.msi）

### LSPサーバー
- [ ] Language Server Protocol実装
- [ ] リアルタイム解析
- [ ] IDE統合の強化

---

## 重要な注意事項

### TODOリストの運用ルール

1. **作業開始前**
   - 必ずこのTODOリストを確認する
   - 作業するタスクを [>] に更新する
   - 依存関係を確認する

2. **作業中**
   - 新しいタスクが見つかったら追加する
   - ブロック事項があれば [!] にして理由を記載
   - 進捗をこまめに更新する

3. **作業完了後**
   - [x] に更新する
   - 関連ドキュメントを更新する
   - テストが通ることを確認する
   - 次のタスクに移る前にコミットする

4. **定期的なレビュー**
   - 週次でTODOリストをレビューする
   - 優先順位を見直す
   - 不要になったタスクを削除する

### コミットメッセージ

TODOアイテムを完了したときのコミットメッセージには、完了したタスクを明記してください。

例:
```
git commit -m "[M1] Implement ArgumentParser

- Completed CLI argument parsing
- Added support for --config, --format options
- Added --help and --version options
- Updated TODO.md

Closes #1"
```

---

**このTODOリストは常に最新の状態を保つこと！**
