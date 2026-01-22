# cclint 開発マイルストーン

## プロジェクト概要

**プロジェクト名**: cclint - Customizable C++ Linter
**開発言語**: C++17
**対象環境**: Ubuntu, macOS
**開発体制**: Claude Code支援による開発

## マイルストーン構成

プロジェクトを6つのマイルストーンに分割し、段階的に機能を実装していきます。

---

## Milestone 0: プロジェクトセットアップ（準備フェーズ）

**目標**: 開発環境の整備とプロジェクト基盤の構築

**期間**: 1-2日

### 成果物
- [x] ディレクトリ構造の作成
- [x] 要件定義書
- [x] 設計書
- [x] 詳細設計書
- [x] マイルストーン計画
- [ ] TODO リスト
- [ ] README.md
- [ ] CLAUDE.md

### タスク
- [x] docs/ ディレクトリの作成
- [x] src/ ディレクトリの作成
- [x] 要件定義の記述
- [x] アーキテクチャ設計の記述
- [x] 詳細設計の記述
- [x] マイルストーンの定義
- [ ] TODOリストの作成
- [ ] README.mdの作成
- [ ] CLAUDE.mdの作成
- [ ] .gitignoreの作成
- [ ] CMakeLists.txtの初期版作成
- [ ] ビルド環境の確認（LLVM/Clang, yaml-cpp, Lua）

### 完了基準
- すべてのドキュメントが完成している
- ビルドシステムの骨格ができている
- 依存関係が明確になっている

---

## Milestone 1: 最小動作版（MVP）

**目標**: コンパイラコマンドをラップし、最小限の解析を行う

**期間**: 3-5日

### 成果物
- CLIパーサー
- 設定ローダー（YAML基本機能）
- コンパイララッパー
- Clangパーサー統合（基本）
- テキスト出力
- 単体テスト

### 主要タスク

#### 1. CLI Module (cli/)
- [ ] ArgumentParser実装
  - [ ] cclintオプション解析
  - [ ] コンパイラコマンド抽出
  - [ ] ヘルプメッセージ表示
  - [ ] バージョン表示
- [ ] コマンドライン引数のテスト

#### 2. Config Module (config/)
- [ ] 基本的なConfig構造体定義
- [ ] ConfigLoader実装（ファイル探索）
- [ ] YAML設定ファイルパーサー（yaml-cpp利用）
- [ ] デフォルト設定の適用
- [ ] 設定の検証

#### 3. Compiler Module (compiler/)
- [ ] CompilerWrapper実装
  - [ ] コマンド実行機能
  - [ ] ソースファイル抽出
  - [ ] コンパイラフラグ抽出
- [ ] コンパイラ検出（gcc/clang）

#### 4. Parser Module (parser/)
- [ ] ClangParser基本実装
  - [ ] Clang libtooling統合
  - [ ] ASTUnit生成
  - [ ] 基本的なエラーハンドリング
- [ ] ASTVisitor基底クラス

#### 5. Output Module (output/)
- [ ] TextFormatter実装
  - [ ] 基本的なテキスト出力
  - [ ] カラー対応
  - [ ] ソースコードスニペット表示

#### 6. Main Entry Point
- [ ] main.cpp実装
  - [ ] 各モジュールの統合
  - [ ] 基本的な実行フロー
  - [ ] エラーハンドリング

#### 7. Build System
- [ ] CMakeLists.txt完成版
- [ ] 依存関係の解決
- [ ] インストールターゲット

#### 8. Documentation & Testing
- [ ] ビルド手順のドキュメント
- [ ] 基本的な使い方のドキュメント
- [ ] 単体テストの追加（Google Test）
- [ ] CI設定（GitHub Actions）

### 完了基準
- `cclint g++ main.cpp` でコンパイラをラップして実行できる
- C++ファイルをパースしてASTを構築できる
- ビルドが成功し、実行可能バイナリが生成される
- 基本的なエラーメッセージが表示される

### テストシナリオ
```bash
# 1. ヘルプ表示
./cclint --help

# 2. バージョン表示
./cclint --version

# 3. 基本的なコンパイララップ
./cclint g++ -std=c++17 test.cpp

# 4. 設定ファイル指定
./cclint --config=test.yaml g++ test.cpp
```

---

## Milestone 2: ルールシステム基盤

**目標**: ルール定義とチェック機能の実装

**期間**: 4-6日

### 成果物
- ルール基底クラス
- ルールレジストリ
- 2-3個の組み込みルール
- 診断レポーター
- YAML設定でのルール有効化

### 主要タスク

#### 1. Rules Module (rules/)
- [ ] RuleBase基底クラス実装
  - [ ] インターフェース定義
  - [ ] 診断生成ヘルパー
- [ ] RuleRegistry実装
  - [ ] ルール登録機能
  - [ ] ルール有効化/無効化
  - [ ] ルール検索
- [ ] RuleExecutor実装
  - [ ] ルール実行ループ
  - [ ] エラーハンドリング

#### 2. Builtin Rules (rules/builtin/)
- [ ] 命名規則チェック（naming_convention）
  - [ ] 関数名チェック
  - [ ] 変数名チェック
  - [ ] クラス名チェック
- [ ] ヘッダーガードチェック（header_guard）
  - [ ] インクルードガード検出
  - [ ] pragma once対応
- [ ] 最大行長チェック（max_line_length）

#### 3. Diagnostic Module (diagnostic/)
- [ ] Diagnostic構造体定義
- [ ] SourceLocation実装
- [ ] DiagnosticReporter実装
  - [ ] 診断の追加
  - [ ] 診断のソート
  - [ ] 統計情報（エラー数など）

#### 4. Engine Module (engine/)
- [ ] AnalysisEngine実装
  - [ ] ファイルごとの解析
  - [ ] ルール実行の統合
  - [ ] ファイルフィルタリング

#### 5. Config Enhancement
- [ ] ルール設定のYAML定義
- [ ] ルールパラメータの設定
- [ ] severity設定

#### 6. Testing
- [ ] 各ルールの単体テスト
- [ ] ルールレジストリのテスト
- [ ] 統合テスト（エンドツーエンド）

### 完了基準
- 2-3個の組み込みルールが動作する
- YAML設定でルールを有効化/無効化できる
- 診断結果が適切にフォーマットされる
- ルールが正しくC++コードをチェックする

### テストシナリオ
```bash
# 1. デフォルトルールでチェック
./cclint g++ test.cpp

# 2. 特定のルールのみ有効化
./cclint --config=rules.yaml g++ test.cpp

# 3. すべてのルールを実行
./cclint g++ --all-rules test.cpp
```

---

## Milestone 3: Lua統合

**目標**: Luaスクリプトによるカスタムルール定義

**期間**: 5-7日

### 成果物
- Luaエンジン
- Lua-C++ブリッジ
- Lua API（AST操作）
- Luaサンドボックス
- サンプルLuaルール

### 主要タスク

#### 1. Lua Module (lua/)
- [ ] LuaEngine実装
  - [ ] Lua VM初期化
  - [ ] スクリプトロード
  - [ ] エラーハンドリング
- [ ] Lua Bridge実装
  - [ ] C++からLua関数呼び出し
  - [ ] LuaからC++関数呼び出し
  - [ ] データ型変換

#### 2. Lua API Implementation
- [ ] 診断報告API
  - [ ] `report_error(location, message)`
  - [ ] `report_warning(location, message)`
  - [ ] `report_info(location, message)`
- [ ] ASTアクセスAPI
  - [ ] `get_node_type(node)`
  - [ ] `get_node_name(node)`
  - [ ] `get_node_location(node)`
  - [ ] `get_children(node)`
- [ ] ユーティリティAPI
  - [ ] `match_pattern(text, regex)`
  - [ ] `get_file_content(path)`
  - [ ] `get_source_range(node)`

#### 3. Lua Sandbox
- [ ] セキュリティ制限
  - [ ] ファイルI/O制限
  - [ ] ネットワークアクセス禁止
  - [ ] システムコマンド実行禁止
- [ ] リソース制限
  - [ ] メモリ使用量制限
  - [ ] 実行時間制限

#### 4. Lua Rule System
- [ ] ルール登録機構
  - [ ] `register_rule(name, function)`
- [ ] ルール設定
  - [ ] パラメータ受け渡し
  - [ ] severity設定

#### 5. Sample Lua Rules
- [ ] サイクロマティック複雑度チェック
- [ ] TODOコメント検出
- [ ] プロジェクト固有ルールの例

#### 6. Documentation
- [ ] Lua API リファレンス
- [ ] Luaルール作成ガイド
- [ ] サンプルコード集

#### 7. Testing
- [ ] Luaエンジンのテスト
- [ ] Lua APIのテスト
- [ ] サンドボックスのテスト
- [ ] サンプルルールのテスト

### 完了基準
- Luaスクリプトからルールを定義できる
- Luaから診断を報告できる
- LuaからASTにアクセスできる
- サンドボックスが安全に動作する
- サンプルLuaルールが動作する

### テストシナリオ
```bash
# 1. Luaスクリプトを読み込む
./cclint --config=with_lua.yaml g++ test.cpp

# 2. 複数のLuaスクリプトを読み込む
./cclint --lua=rule1.lua --lua=rule2.lua g++ test.cpp

# 3. Luaルールのエラーハンドリング
./cclint --lua=broken_rule.lua g++ test.cpp
```

---

## Milestone 4: パフォーマンス最適化

**目標**: 大規模プロジェクトでの実用性向上

**期間**: 3-5日

### 成果物
- 並列処理実装
- キャッシュ機構
- インクリメンタル解析
- パフォーマンス計測ツール

### 主要タスク

#### 1. Parallel Processing
- [ ] マルチスレッド解析
  - [ ] ファイル並列処理
  - [ ] スレッドプール実装
  - [ ] CPU コア数自動検出
- [ ] スレッドセーフ対策
  - [ ] 診断レポーターのロック
  - [ ] ルールレジストリのロック

#### 2. Caching System
- [ ] パース結果キャッシュ
  - [ ] ファイルハッシュ計算
  - [ ] キャッシュの保存/読み込み
  - [ ] キャッシュの無効化
- [ ] キャッシュストレージ
  - [ ] ディスクベースキャッシュ
  - [ ] キャッシュディレクトリ管理

#### 3. Incremental Analysis
- [ ] 変更ファイル検出
  - [ ] gitとの統合
  - [ ] タイムスタンプチェック
- [ ] 部分解析

#### 4. Performance Monitoring
- [ ] 解析時間計測
- [ ] メモリ使用量計測
- [ ] ボトルネック分析
- [ ] プロファイリングツール統合

#### 5. Optimization
- [ ] ASTのメモリ最適化
- [ ] 不要なコピーの削減
- [ ] 早期終了の実装

#### 6. Testing
- [ ] パフォーマンステスト
- [ ] 大規模プロジェクトでのテスト
- [ ] ベンチマークスイート

### 完了基準
- 10,000行のコードを10秒以内に解析
- 並列処理が正しく動作
- キャッシュが有効に機能
- メモリ使用量が1GB未満

### テストシナリオ
```bash
# 1. 並列処理の有効化
./cclint --threads=4 g++ *.cpp

# 2. キャッシュの有効化
./cclint --enable-cache g++ *.cpp

# 3. インクリメンタル解析
./cclint --incremental g++ *.cpp

# 4. パフォーマンス計測
./cclint --profile g++ *.cpp
```

---

## Milestone 5: 出力フォーマットとツール統合

**目標**: CI/CDとIDE統合のための出力形式サポート

**期間**: 3-4日

### 成果物
- JSON出力フォーマッター
- XML出力フォーマッター
- CI/CD統合ガイド
- IDE統合サンプル

### 主要タスク

#### 1. JSON Formatter
- [ ] JsonFormatter実装
  - [ ] 診断のJSON変換
  - [ ] 構造化データ出力
  - [ ] JSON Schemaの定義
- [ ] Pretty print対応

#### 2. XML Formatter
- [ ] XmlFormatter実装
  - [ ] 診断のXML変換
  - [ ] DTD/Schema定義
- [ ] 既存フォーマット互換性（checkstyle等）

#### 3. Enhanced Text Formatter
- [ ] カラー出力の改善
- [ ] ソースコンテキスト表示
- [ ] 修正提案の表示
- [ ] 統計情報サマリー

#### 4. CI/CD Integration
- [ ] GitHub Actions統合例
- [ ] GitLab CI統合例
- [ ] Jenkins統合例
- [ ] 終了コードの制御

#### 5. IDE Integration Samples
- [ ] VSCode統合サンプル
  - [ ] tasks.json
  - [ ] problem matcher
- [ ] Vim/Neovim統合
  - [ ] ALE統合
  - [ ] quickfix対応

#### 6. Documentation
- [ ] CI/CD統合ガイド
- [ ] IDE統合ガイド
- [ ] 出力フォーマットリファレンス

#### 7. Testing
- [ ] 各フォーマッターのテスト
- [ ] CI環境でのテスト
- [ ] IDE統合のテスト

### 完了基準
- JSON/XML出力が正しく動作
- CI/CDパイプラインに統合できる
- IDEで診断結果を表示できる
- 各フォーマットのドキュメントが完備

### テストシナリオ
```bash
# 1. JSON出力
./cclint --format=json g++ test.cpp > results.json

# 2. XML出力
./cclint --format=xml g++ test.cpp > results.xml

# 3. CI環境での実行
./cclint --format=json --max-errors=10 g++ *.cpp
```

---

## Milestone 6: リリース準備

**目標**: v1.0.0リリースに向けた最終調整

**期間**: 3-5日

### 成果物
- 完全なドキュメント
- インストーラー
- パッケージ（deb, brew formula）
- リリースノート

### 主要タスク

#### 1. Documentation
- [ ] README.md完成版
- [ ] インストールガイド
- [ ] ユーザーガイド
- [ ] API リファレンス
- [ ] トラブルシューティングガイド
- [ ] FAQs
- [ ] CHANGELOGの作成

#### 2. Examples & Samples
- [ ] サンプルプロジェクト
- [ ] YAML設定サンプル集
- [ ] Luaルールサンプル集
- [ ] チュートリアル

#### 3. Packaging
- [ ] Debian パッケージ (.deb)
  - [ ] パッケージ作成スクリプト
  - [ ] 依存関係の定義
- [ ] Homebrew Formula
  - [ ] formula作成
  - [ ] テスト
- [ ] ソースtarball
  - [ ] リリースアーカイブ作成

#### 4. Installation
- [ ] インストールスクリプト
- [ ] アンインストールスクリプト
- [ ] 依存関係の自動インストール

#### 5. Quality Assurance
- [ ] すべてのテストの実行
- [ ] コードカバレッジ確認（目標80%以上）
- [ ] メモリリーク検査（valgrind）
- [ ] 静的解析（clang-tidy, cppcheck）
- [ ] ドキュメントレビュー

#### 6. Release Process
- [ ] バージョン番号の設定
- [ ] リリースノートの作成
- [ ] GitHubリリースの作成
- [ ] タグの作成

#### 7. Website & Community
- [ ] プロジェクトWebサイト（GitHub Pages）
- [ ] ロゴ作成
- [ ] コントリビューションガイドライン
- [ ] Code of Conduct
- [ ] Issue/PR テンプレート

### 完了基準
- すべてのドキュメントが完成
- Ubuntu/macOSで簡単にインストールできる
- すべてのテストが通過
- リリースノートが完成
- バージョン1.0.0としてリリース準備完了

### テストシナリオ
```bash
# 1. パッケージからのインストール（Ubuntu）
sudo dpkg -i cclint_1.0.0_amd64.deb
cclint --version

# 2. Homebrewからのインストール（macOS）
brew install cclint
cclint --version

# 3. ソースからのビルド
tar xzf cclint-1.0.0.tar.gz
cd cclint-1.0.0
mkdir build && cd build
cmake ..
make
sudo make install

# 4. 基本的な動作確認
cclint g++ -std=c++17 test.cpp
```

---

## タイムライン概要

```
Week 1:  M0 (Setup) + M1 (MVP) 開始
Week 2:  M1 (MVP) 完成 + M2 (Rules) 開始
Week 3:  M2 (Rules) 完成 + M3 (Lua) 開始
Week 4:  M3 (Lua) 継続
Week 5:  M3 (Lua) 完成 + M4 (Performance) 開始
Week 6:  M4 (Performance) 完成 + M5 (Output) 開始
Week 7:  M5 (Output) 完成 + M6 (Release) 開始
Week 8:  M6 (Release) 完成 → v1.0.0 リリース
```

**総開発期間**: 約6-8週間

---

## 成功指標（KPI）

### 機能面
- [ ] すべての要件が実装されている
- [ ] 2-3個の組み込みルールが動作
- [ ] Luaによるカスタムルールが動作
- [ ] YAML設定が正しく読み込まれる
- [ ] 3つの出力フォーマット対応（text, json, xml）

### パフォーマンス面
- [ ] 10,000行のコードを10秒以内に解析
- [ ] 並列処理で2倍以上の高速化
- [ ] メモリ使用量1GB未満
- [ ] キャッシュで50%以上の高速化

### 品質面
- [ ] テストカバレッジ80%以上
- [ ] メモリリークなし
- [ ] クラッシュなし
- [ ] ドキュメント完備

### ユーザビリティ面
- [ ] 10分以内にインストール完了
- [ ] 30分以内にカスタムルール作成可能
- [ ] CI/CDに5分以内に統合可能
- [ ] 明確なエラーメッセージ

---

## リスク管理

### 技術リスク
1. **Clang/LLVM統合の複雑さ**
   - 対策: 早期にプロトタイプを作成し検証

2. **Luaサンドボックスのセキュリティ**
   - 対策: セキュリティレビューとテスト強化

3. **パフォーマンス目標未達**
   - 対策: 早期にベンチマーク環境を構築

### スケジュールリスク
1. **依存ライブラリのビルド問題**
   - 対策: 環境構築の自動化

2. **想定外の技術課題**
   - 対策: 各マイルストーンにバッファを含める

### リソースリスク
1. **開発リソース不足**
   - 対策: スコープの調整、優先順位の明確化

---

## 次のステップ

1. **Milestone 0の完了**
   - TODOリストの作成
   - README.mdの作成
   - CLAUDE.mdの作成

2. **開発環境のセットアップ**
   - LLVM/Clangのインストール確認
   - yaml-cppのインストール確認
   - Luaのダウンロード

3. **Milestone 1の開始**
   - CMakeLists.txtの作成
   - 最初のコード実装
