# cclint 要件定義書

## 1. プロジェクト概要

### 1.1 プロジェクト名
cclint - Customizable C++ Linter

### 1.2 目的
C++コードの静的解析を行うカスタマイズ可能なlinterツールを開発する。
ユーザーは独自のリントルールを定義でき、コンパイルなしでルールを追加・変更できることを目指す。

### 1.3 背景
- 既存のlinterツールは固定されたルールセットを持つことが多い
- プロジェクト固有のコーディング規約をチェックするには、既存ツールのカスタマイズが困難
- コンパイルが必要なカスタムルール追加は開発速度を低下させる
- CI/CDパイプラインに簡単に統合できる軽量なツールが必要

## 2. 機能要件

### 2.1 コア機能

#### 2.1.1 コンパイラコマンドラッピング
- gccやclangのコマンドをラップして実行できる
- 構文:
  ```
  cclint g++ [compiler options] source.cpp
  cclint [cclint options] g++ [compiler options] source.cpp
  cclint --config=path/to/config.yaml clang++ -std=c++17 main.cpp
  ```
- 元のコンパイラの出力とcclintの警告を両方表示

#### 2.1.2 C++ソースコード解析
- C++初版からC++最新バージョン（C++23/26）までのすべての標準に対応
- ソースコードをパースし、抽象構文木(AST)を構築
- トークン、式、文、宣言、定義を識別

#### 2.1.3 カスタムリントルール
- デフォルトではリントルールなし（空の状態）
- ユーザーが独自のルールを定義する仕組みを提供
- 2つの設定方法:
  1. YAML設定ファイル（cclint.yaml）
  2. Luaスクリプトによる動的ルール定義

### 2.2 設定システム

#### 2.2.1 YAML設定ファイル（cclint.yaml）
用途: 静的な設定とシンプルなパターンマッチングルール

設定項目:
- `version`: 設定ファイルのバージョン
- `rules`: 有効化するルールのリスト
- `severity`: ルールの深刻度（error, warning, info）
- `include_patterns`: 解析対象のファイルパターン
- `exclude_patterns`: 解析から除外するファイルパターン
- `compiler`: ラップするコンパイラの設定
- `lua_scripts`: 読み込むLuaスクリプトのパス
- `output`: 出力形式（text, json, xml）
- `max_errors`: エラー数の上限
- `cpp_standard`: 対象のC++標準（cpp11, cpp14, cpp17, cpp20, cpp23, auto）

例:
```yaml
version: 1.0
cpp_standard: cpp17
severity: warning
include_patterns:
  - "src/**/*.cpp"
  - "include/**/*.hpp"
exclude_patterns:
  - "test/**"
  - "third_party/**"
rules:
  - naming_conventions
  - header_guards
output: text
max_errors: 100
lua_scripts:
  - ".cclint/custom_rules.lua"
compiler:
  pass_through: true
  show_compiler_output: true
```

#### 2.2.2 Lua設定スクリプト
用途: 複雑なロジックを持つルール、動的なルール定義

機能:
- ASTノードへのアクセス
- カスタムチェック関数の定義
- 正規表現パターンマッチング
- ファイル情報へのアクセス
- 条件付きルール適用

Lua API例:
```lua
-- ルール定義
function check_function_name(node)
  if node.type == "FunctionDecl" then
    local name = node.name
    if not name:match("^[a-z][a-zA-Z0-9]*$") then
      report_error(node.location, "Function name should be camelCase")
    end
  end
end

-- ルール登録
register_rule("function_naming", check_function_name)
```

#### 2.2.3 設定ファイルの優先順位
1. コマンドライン引数（`--config`）
2. カレントディレクトリの `.cclint.yaml`
3. プロジェクトルートの `cclint.yaml`
4. ホームディレクトリの `~/.cclint/config.yaml`
5. デフォルト設定（ルールなし）

### 2.3 出力機能

#### 2.3.1 出力形式
- テキスト形式（人間が読みやすい）
- JSON形式（CI/CDツール連携用）
- XML形式（IDEプラグイン用）

#### 2.3.2 出力内容
- ファイル名と行番号
- 問題の深刻度（error, warning, info）
- ルール名
- 問題の説明
- ソースコードの該当箇所（コンテキスト表示）
- 修正提案（可能な場合）

### 2.4 パフォーマンス要件
- 大規模プロジェクト（10,000ファイル以上）でも実用的な速度
- マルチスレッド対応によるファイル並列処理
- インクリメンタル解析（変更されたファイルのみ再解析）
- キャッシュ機構（解析結果の保存と再利用）

## 3. 非機能要件

### 3.1 対応環境

#### 3.1.1 オペレーティングシステム
- Ubuntu 20.04 LTS以降
- macOS 11 (Big Sur)以降

#### 3.1.2 C++標準
- C++98/03
- C++11
- C++14
- C++17（開発言語）
- C++20
- C++23
- C++26（ドラフト対応）

#### 3.1.3 コンパイラ
- GCC 7.0以降
- Clang 10.0以降

### 3.2 依存関係
- Clang/LLVMライブラリ（C++パーサーとして使用）
- libyaml-cpp（YAML解析）
- Lua 5.4以降（スクリプトエンジン）
- 標準C++17ライブラリのみ（その他の実行時依存なし）

### 3.3 インストール
- パッケージマネージャ対応（apt, brew）
- ソースからのビルド対応
- シングルバイナリでの配布

### 3.4 互換性
- 既存のビルドシステムとの統合（Make, CMake, Ninja, Bazel）
- CI/CDツールとの統合（GitHub Actions, GitLab CI, Jenkins）
- エディタ/IDE統合（VSCode, Vim, Emacs）

### 3.5 保守性
- 明確なドキュメント
- テストカバレッジ80%以上
- モジュール構造による拡張性

## 4. ユースケース

### 4.1 基本的な使用
1. ユーザーがcclintをインストール
2. プロジェクトルートに `cclint.yaml` を作成
3. ルールを定義
4. `cclint g++ main.cpp` を実行
5. リント警告とコンパイル結果を確認

### 4.2 CI/CD統合
1. リポジトリに `.cclint.yaml` を追加
2. CI設定ファイルでcclintを実行
3. リント違反があればビルドを失敗させる

### 4.3 複雑なカスタムルール
1. `.cclint/custom_rules.lua` を作成
2. Luaで複雑なチェックロジックを実装
3. `cclint.yaml` でLuaスクリプトを読み込み
4. プロジェクト固有のルールを適用

## 5. 制約事項

### 5.1 技術的制約
- プリプロセッサマクロの展開は限定的
- テンプレートメタプログラミングの完全な解析は困難
- コンパイル時定数評価は限定的

### 5.2 スコープ外
- コードフォーマッティング（clang-formatの役割）
- 自動修正機能（初期バージョンでは提案のみ）
- Windows対応（将来的な拡張として検討）
- 動的解析・実行時チェック

## 6. 将来的な拡張

### 6.1 フェーズ2（v2.0）
- 自動修正機能（fixitヒント）
- より詳細なデータフロー解析
- プラグインシステム（共有ライブラリとしてのルール）
- WebAssembly対応（ブラウザで動作）

### 6.2 フェーズ3（v3.0）
- Windows対応
- グラフィカルユーザーインターフェース
- LSP（Language Server Protocol）サーバー実装
- クラウドベースのルール共有プラットフォーム

## 7. 成功基準

### 7.1 定量的基準
- 10,000行のC++コードを10秒以内に解析
- メモリ使用量が1GBを超えない
- ユーザー定義ルールの作成が30分以内に可能
- CI/CDパイプラインへの統合が5分以内に完了

### 7.2 定性的基準
- ユーザーがルールを簡単に追加できる
- 既存のビルドプロセスに透過的に統合できる
- エラーメッセージが明確で理解しやすい
- ドキュメントが充実しており、サンプルが豊富
