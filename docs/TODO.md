# cclint TODO リスト

**最終更新**: 2026-01-22

このドキュメントはプロジェクトの進捗を管理するためのTODOリストです。
作業を開始する前に必ずこのリストを確認し、作業が完了したら更新してください。

## 凡例
- [ ] 未着手
- [>] 作業中
- [x] 完了
- [!] ブロック中（依存関係待ち）
- [~] 保留

---

## Milestone 0: プロジェクトセットアップ

### ドキュメント作成
- [x] ディレクトリ構造の作成（docs/, src/）
- [x] 要件定義書の作成（docs/requirements.md）
- [x] 設計書の作成（docs/design.md）
- [x] 詳細設計書の作成（docs/detailed_design.md）
- [x] マイルストーンの作成（docs/milestones.md）
- [>] TODOリストの作成（docs/TODO.md）
- [ ] README.mdの作成
- [ ] CLAUDE.mdの作成
  - [ ] TODOリスト運用方法の明記
  - [ ] 開発ガイドラインの記述
  - [ ] Claude Code利用方法の記述

### プロジェクト設定
- [ ] .gitignoreの作成
- [ ] .clang-formatの作成
- [ ] .clang-tidyの作成
- [ ] LICENSEファイルの追加

### ビルドシステム
- [ ] CMakeLists.txt（ルート）の作成
- [ ] CMakeLists.txt（src/）の作成
- [ ] CMakeLists.txt（tests/）の作成
- [ ] 依存関係の設定
  - [ ] LLVM/Clangの検出
  - [ ] yaml-cppの検出
  - [ ] Lua 5.4の組み込み

### 環境確認
- [ ] Ubuntu環境でのビルド確認
- [ ] macOS環境でのビルド確認
- [ ] 依存ライブラリのバージョン確認

---

## Milestone 1: 最小動作版（MVP）

### 1. CLI Module (src/cli/)

#### ArgumentParser
- [ ] argument_parser.hpp の作成
- [ ] argument_parser.cpp の実装
  - [ ] コマンドライン引数の基本解析
  - [ ] cclintオプションの抽出
  - [ ] コンパイラコマンドの抽出
  - [ ] `--config` オプション対応
  - [ ] `--format` オプション対応
  - [ ] `-v/--verbose` オプション対応
  - [ ] `-q/--quiet` オプション対応
  - [ ] `--help` オプション対応
  - [ ] `--version` オプション対応
- [ ] help_formatter.hpp の作成
- [ ] help_formatter.cpp の実装
  - [ ] ヘルプメッセージの生成
  - [ ] 使用例の表示
  - [ ] オプション一覧の表示
- [ ] 単体テスト（tests/cli/test_argument_parser.cpp）

### 2. Config Module (src/config/)

#### データ構造
- [ ] config_types.hpp の作成
  - [ ] Severity列挙型の定義
  - [ ] RuleConfig構造体の定義
  - [ ] Config構造体の定義

#### ConfigLoader
- [ ] config_loader.hpp の作成
- [ ] config_loader.cpp の実装
  - [ ] 設定ファイルの探索
    - [ ] カレントディレクトリ (.cclint.yaml)
    - [ ] カレントディレクトリ (cclint.yaml)
    - [ ] プロジェクトルート (cclint.yaml)
    - [ ] ホームディレクトリ (~/.cclint/config.yaml)
  - [ ] 設定ファイルの読み込み
  - [ ] デフォルト値の適用
  - [ ] 設定の検証

#### YAML Parser
- [ ] yaml_config.hpp の作成
- [ ] yaml_config.cpp の実装
  - [ ] yaml-cppを使ったパース
  - [ ] version フィールドの読み込み
  - [ ] cpp_standard フィールドの読み込み
  - [ ] rules フィールドの読み込み
  - [ ] include_patterns フィールドの読み込み
  - [ ] exclude_patterns フィールドの読み込み
  - [ ] lua_scripts フィールドの読み込み
  - [ ] output フィールドの読み込み
  - [ ] パースエラーのハンドリング
- [ ] 単体テスト（tests/config/test_config_loader.cpp）

### 3. Compiler Module (src/compiler/)

#### CompilerWrapper
- [ ] wrapper.hpp の作成
- [ ] wrapper.cpp の実装
  - [ ] コマンド実行機能
    - [ ] popen を使った実行
    - [ ] 標準出力のキャプチャ
    - [ ] 標準エラー出力のキャプチャ
    - [ ] 終了コードの取得
  - [ ] ソースファイルの抽出
    - [ ] .cpp, .cc, .cxx ファイルの検出
    - [ ] .c ファイルの検出
    - [ ] .h, .hpp ファイルの検出
  - [ ] コンパイラフラグの抽出
    - [ ] -I, -D, -std フラグの抽出
    - [ ] その他のフラグの抽出
- [ ] detector.hpp の作成
- [ ] detector.cpp の実装
  - [ ] gccの検出
  - [ ] clangの検出
  - [ ] コンパイラバージョンの取得
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
- [ ] diagnostic.hpp の作成
  - [ ] Severity列挙型
  - [ ] SourceLocation構造体
  - [ ] Diagnostic構造体
- [ ] reporter.hpp の作成
- [ ] reporter.cpp の実装
  - [ ] 診断の追加
  - [ ] 診断のソート
  - [ ] エラー数/警告数の集計
- [ ] 単体テスト（tests/diagnostic/test_reporter.cpp）

### 6. Output Module (src/output/)

#### TextFormatter
- [ ] formatter.hpp の作成（基底クラス）
- [ ] text_formatter.hpp の作成
- [ ] text_formatter.cpp の実装
  - [ ] 基本的なテキスト出力
  - [ ] カラー出力（ANSIエスケープシーケンス）
  - [ ] ソースコードスニペットの表示
  - [ ] severity の表示
  - [ ] ファイル名:行番号:列番号 の表示
- [ ] create_formatter 関数の実装
- [ ] 単体テスト（tests/output/test_text_formatter.cpp）

### 7. Utility Module (src/util/)

#### ファイルユーティリティ
- [ ] file_utils.hpp の作成
- [ ] file_utils.cpp の実装
  - [ ] ファイル読み込み
  - [ ] ファイル存在チェック
  - [ ] ディレクトリ操作
  - [ ] パターンマッチング

#### 文字列ユーティリティ
- [ ] string_utils.hpp の作成
- [ ] string_utils.cpp の実装
  - [ ] 文字列分割
  - [ ] trim
  - [ ] 正規表現ヘルパー

#### ロガー
- [ ] logger.hpp の作成
- [ ] logger.cpp の実装
  - [ ] ログレベル設定
  - [ ] ログ出力（stdout/stderr）
  - [ ] デバッグログ

### 8. Main Entry Point

#### main.cpp
- [ ] main.cpp の作成
  - [ ] 基本的な実行フロー
    - [ ] 引数解析
    - [ ] 設定読み込み
    - [ ] ソースファイル抽出
    - [ ] パース（AST構築）
    - [ ] コンパイラ実行（オプション）
    - [ ] 結果出力
  - [ ] エラーハンドリング
    - [ ] try-catch
    - [ ] 終了コードの決定
  - [ ] --help, --version の処理

### 9. Build & Testing

#### ビルドシステム
- [ ] CMakeLists.txtの完成
- [ ] ビルドスクリプトの作成（build.sh）
- [ ] インストールターゲットの作成

#### テスト
- [ ] Google Testの統合
- [ ] テストディレクトリの作成（tests/）
- [ ] 統合テスト（tests/integration/）
  - [ ] 基本的な実行テスト
  - [ ] エラーハンドリングのテスト

#### CI/CD
- [ ] GitHub Actions設定（.github/workflows/ci.yml）
  - [ ] Ubuntu環境でのビルド
  - [ ] macOS環境でのビルド
  - [ ] テスト実行
  - [ ] コードカバレッジ

### 10. Documentation

- [ ] ビルド手順のドキュメント（docs/build.md）
- [ ] 基本的な使い方（docs/usage.md）
- [ ] トラブルシューティング（docs/troubleshooting.md）

---

## Milestone 2: ルールシステム基盤

### 1. Rules Module (src/rules/)

#### 基底クラス
- [ ] rule_base.hpp の作成
  - [ ] RuleBaseインターフェース
  - [ ] 診断生成ヘルパー
- [ ] rule_base.cpp の実装

#### RuleRegistry
- [ ] rule_registry.hpp の作成
- [ ] rule_registry.cpp の実装
  - [ ] シングルトンパターン
  - [ ] ルール登録
  - [ ] ルール有効化/無効化
  - [ ] ルール検索
  - [ ] 全ルール名の取得
- [ ] 単体テスト（tests/rules/test_rule_registry.cpp）

#### RuleExecutor
- [ ] rule_executor.hpp の作成
- [ ] rule_executor.cpp の実装
  - [ ] ルール実行ループ
  - [ ] エラーハンドリング
  - [ ] タイムアウト処理
- [ ] 単体テスト（tests/rules/test_rule_executor.cpp）

### 2. Builtin Rules (src/rules/builtin/)

#### 命名規則ルール
- [ ] naming_convention.hpp の作成
- [ ] naming_convention.cpp の実装
  - [ ] 関数名チェック（camelCase）
  - [ ] 変数名チェック
  - [ ] クラス名チェック（PascalCase）
  - [ ] 定数名チェック（UPPER_CASE）
- [ ] 単体テスト（tests/rules/builtin/test_naming_convention.cpp）

#### ヘッダーガードルール
- [ ] header_guard.hpp の作成
- [ ] header_guard.cpp の実装
  - [ ] #ifndef/#define/#endifの検出
  - [ ] #pragma onceの検出
  - [ ] ガード名のチェック
- [ ] 単体テスト（tests/rules/builtin/test_header_guard.cpp）

#### 最大行長ルール
- [ ] max_line_length.hpp の作成
- [ ] max_line_length.cpp の実装
  - [ ] 行長のチェック
  - [ ] 設定可能な最大長
- [ ] 単体テスト（tests/rules/builtin/test_max_line_length.cpp）

### 3. Engine Module (src/engine/)

#### AnalysisEngine
- [ ] analysis_engine.hpp の作成
- [ ] analysis_engine.cpp の実装
  - [ ] ファイルごとの解析
  - [ ] ルール実行の統合
  - [ ] ファイルフィルタリング（include/exclude）
  - [ ] エラーハンドリング
- [ ] 単体テスト（tests/engine/test_analysis_engine.cpp）

### 4. Config Enhancement

- [ ] ルール設定のYAML定義サポート
- [ ] ルールパラメータの設定
- [ ] severity設定（error/warning/info）
- [ ] ルールごとの有効化/無効化

### 5. Testing & Integration

- [ ] 統合テスト（tests/integration/）
  - [ ] 複数ルールの実行
  - [ ] 設定ファイルとの統合
  - [ ] エンドツーエンドテスト
- [ ] サンプルC++ファイルの作成（tests/samples/）

---

## Milestone 3: Lua統合

### 1. Lua Module (src/lua/)

#### LuaEngine
- [ ] lua_engine.hpp の作成
- [ ] lua_engine.cpp の実装
  - [ ] Lua VMの初期化
  - [ ] スクリプトのロード
  - [ ] エラーハンドリング
  - [ ] リソース管理（デストラクタ）
- [ ] 単体テスト（tests/lua/test_lua_engine.cpp）

#### Lua Bridge
- [ ] lua_bridge.hpp の作成
- [ ] lua_bridge.cpp の実装
  - [ ] C++からLua関数の呼び出し
  - [ ] LuaからC++関数の呼び出し
  - [ ] データ型変換（C++ ⇔ Lua）
  - [ ] ASTノードのLua公開
- [ ] 単体テスト（tests/lua/test_lua_bridge.cpp）

### 2. Lua API Implementation (src/lua/lua_api.cpp)

#### 診断報告API
- [ ] report_error の実装
- [ ] report_warning の実装
- [ ] report_info の実装

#### ASTアクセスAPI
- [ ] get_node_type の実装
- [ ] get_node_name の実装
- [ ] get_node_location の実装
- [ ] get_children の実装
- [ ] get_parent の実装

#### ユーティリティAPI
- [ ] match_pattern の実装（正規表現）
- [ ] get_file_content の実装
- [ ] get_source_range の実装

### 3. Lua Sandbox

#### セキュリティ制限
- [ ] ファイルI/O制限の実装
  - [ ] io.open の制限
  - [ ] io.popen の無効化
- [ ] ネットワークアクセス禁止
  - [ ] socket ライブラリの無効化
- [ ] システムコマンド実行禁止
  - [ ] os.execute の無効化
  - [ ] os.exit の無効化

#### リソース制限
- [ ] メモリ使用量制限
- [ ] 実行時間制限（タイムアウト）
- [ ] スタック深度制限

### 4. Lua Rule System

#### ルール登録
- [ ] register_rule 関数の実装
- [ ] ルール関数の呼び出し
- [ ] ルール実行エラーのハンドリング

#### ルール設定
- [ ] パラメータの受け渡し
- [ ] severity設定
- [ ] ルールの有効化/無効化

### 5. Sample Lua Rules

- [ ] サイクロマティック複雑度チェック（examples/rules/complexity.lua）
- [ ] TODOコメント検出（examples/rules/todo_detector.lua）
- [ ] 使用例ルール（examples/rules/example_rule.lua）

### 6. Documentation

- [ ] Lua API リファレンス（docs/lua_api.md）
- [ ] Luaルール作成ガイド（docs/creating_lua_rules.md）
- [ ] サンプルコード集（docs/lua_examples.md）

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

## Milestone 5: 出力フォーマットとツール統合

### 1. JSON Formatter

- [ ] json_formatter.hpp の作成
- [ ] json_formatter.cpp の実装
  - [ ] 診断のJSON変換
  - [ ] JSON Schemaの定義
  - [ ] Pretty print対応
- [ ] 単体テスト

### 2. XML Formatter

- [ ] xml_formatter.hpp の作成
- [ ] xml_formatter.cpp の実装
  - [ ] 診断のXML変換
  - [ ] DTD/Schemaの定義
  - [ ] checkstyle形式対応
- [ ] 単体テスト

### 3. Enhanced Text Formatter

- [ ] カラー出力の改善
- [ ] ソースコンテキスト表示の改善
- [ ] 修正提案の表示
- [ ] 統計情報サマリー

### 4. CI/CD Integration

- [ ] GitHub Actions統合例（examples/ci/github-actions.yml）
- [ ] GitLab CI統合例（examples/ci/gitlab-ci.yml）
- [ ] Jenkins統合例（examples/ci/Jenkinsfile）
- [ ] 終了コードの制御

### 5. IDE Integration

#### VSCode
- [ ] tasks.json サンプル
- [ ] problem matcher定義
- [ ] 統合ガイド

#### Vim/Neovim
- [ ] ALE統合サンプル
- [ ] quickfix対応
- [ ] 統合ガイド

### 6. Documentation

- [ ] CI/CD統合ガイド（docs/ci_integration.md）
- [ ] IDE統合ガイド（docs/ide_integration.md）
- [ ] 出力フォーマットリファレンス（docs/output_formats.md）

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
