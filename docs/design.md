# cclint アーキテクチャ設計書

## 1. システムアーキテクチャ

### 1.1 全体構成

```
┌─────────────────────────────────────────────────────────────┐
│                    cclint メインプロセス                      │
│                                                               │
│  ┌─────────────┐   ┌──────────────┐   ┌─────────────────┐  │
│  │ CLI Parser  │──→│ Config Loader│──→│ Compiler Wrapper│  │
│  └─────────────┘   └──────────────┘   └─────────────────┘  │
│                            │                     │           │
│                            ↓                     ↓           │
│                    ┌───────────────┐    ┌──────────────┐    │
│                    │ Rule Registry │    │   Compiler   │    │
│                    └───────────────┘    │   (g++/clang)│    │
│                            │             └──────────────┘    │
│                            ↓                                 │
│  ┌──────────────────────────────────────────────────┐       │
│  │           Source Analysis Pipeline               │       │
│  │                                                   │       │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────┐  │       │
│  │  │ Lexer    │→ │ Parser   │→ │ AST Builder  │  │       │
│  │  └──────────┘  └──────────┘  └──────────────┘  │       │
│  │                                      │           │       │
│  │                                      ↓           │       │
│  │              ┌───────────────────────────┐      │       │
│  │              │   Rule Engine             │      │       │
│  │              │  - YAML Rules             │      │       │
│  │              │  - Lua Rules              │      │       │
│  │              └───────────────────────────┘      │       │
│  │                          │                       │       │
│  │                          ↓                       │       │
│  │              ┌───────────────────────────┐      │       │
│  │              │  Diagnostic Reporter      │      │       │
│  │              └───────────────────────────┘      │       │
│  └──────────────────────────────────────────────────┘       │
│                            │                                 │
│                            ↓                                 │
│                    ┌───────────────┐                        │
│                    │ Output        │                        │
│                    │ Formatter     │                        │
│                    │ (text/json/xml)│                       │
│                    └───────────────┘                        │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 モジュール構成

```
cclint/
├── src/
│   ├── main.cpp                    # エントリーポイント
│   ├── cli/
│   │   ├── argument_parser.hpp    # コマンドライン引数解析
│   │   ├── argument_parser.cpp
│   │   └── help_formatter.cpp     # ヘルプメッセージ生成
│   ├── config/
│   │   ├── config_loader.hpp      # 設定ファイル読み込み
│   │   ├── config_loader.cpp
│   │   ├── yaml_config.hpp        # YAML設定処理
│   │   ├── yaml_config.cpp
│   │   └── config_types.hpp       # 設定データ構造
│   ├── compiler/
│   │   ├── wrapper.hpp            # コンパイララッパー
│   │   ├── wrapper.cpp
│   │   └── detector.cpp           # コンパイラ検出
│   ├── parser/
│   │   ├── lexer.hpp              # 字句解析器
│   │   ├── lexer.cpp
│   │   ├── parser.hpp             # 構文解析器
│   │   ├── parser.cpp
│   │   ├── ast.hpp                # AST定義
│   │   ├── ast.cpp
│   │   └── clang_parser.cpp       # Clang API利用パーサー
│   ├── rules/
│   │   ├── rule_base.hpp          # ルール基底クラス
│   │   ├── rule_registry.hpp      # ルール登録管理
│   │   ├── rule_registry.cpp
│   │   ├── yaml_rule.hpp          # YAML定義ルール
│   │   ├── yaml_rule.cpp
│   │   └── builtin/               # 組み込みルール例
│   │       ├── naming_convention.cpp
│   │       └── header_guard.cpp
│   ├── lua/
│   │   ├── lua_engine.hpp         # Luaエンジン
│   │   ├── lua_engine.cpp
│   │   ├── lua_bridge.hpp         # C++ <-> Lua ブリッジ
│   │   ├── lua_bridge.cpp
│   │   └── lua_api.cpp            # Lua API実装
│   ├── engine/
│   │   ├── analysis_engine.hpp    # 解析エンジン
│   │   ├── analysis_engine.cpp
│   │   ├── rule_executor.hpp      # ルール実行器
│   │   └── rule_executor.cpp
│   ├── diagnostic/
│   │   ├── diagnostic.hpp         # 診断結果データ型
│   │   ├── reporter.hpp           # 診断レポーター
│   │   └── reporter.cpp
│   ├── output/
│   │   ├── formatter.hpp          # 出力フォーマッター基底
│   │   ├── text_formatter.cpp     # テキスト出力
│   │   ├── json_formatter.cpp     # JSON出力
│   │   └── xml_formatter.cpp      # XML出力
│   └── util/
│       ├── file_utils.hpp         # ファイル操作ユーティリティ
│       ├── file_utils.cpp
│       ├── string_utils.hpp       # 文字列操作
│       ├── string_utils.cpp
│       └── logger.hpp             # ロギング
└── docs/
    └── (各種ドキュメント)
```

## 2. コンポーネント設計

### 2.1 CLI Parser
**責務**: コマンドライン引数の解析と検証

**入力**:
- argc, argv（コマンドライン引数）

**出力**:
- ParsedArguments構造体
  - cclintオプション
  - コンパイラコマンド
  - ソースファイルリスト

**主要クラス**:
```cpp
class ArgumentParser {
public:
    struct ParsedArguments {
        std::string config_file;
        std::vector<std::string> compiler_command;
        std::vector<std::string> source_files;
        std::string output_format;
        int verbosity;
    };

    ParsedArguments parse(int argc, char** argv);
};
```

### 2.2 Config Loader
**責務**: 設定ファイルの読み込みと統合

**主要機能**:
- YAML設定ファイルの読み込み
- 設定ファイルの探索（優先順位に従う）
- デフォルト値の適用
- 環境変数の展開

**主要クラス**:
```cpp
class ConfigLoader {
public:
    struct Config {
        std::string version;
        std::string cpp_standard;
        std::vector<std::string> include_patterns;
        std::vector<std::string> exclude_patterns;
        std::vector<std::string> enabled_rules;
        std::map<std::string, std::string> rule_config;
        std::vector<std::string> lua_scripts;
        std::string output_format;
        int max_errors;
        bool show_compiler_output;
    };

    Config load(const std::string& config_path = "");
private:
    Config find_and_load_config();
    void merge_configs(Config& base, const Config& override);
};
```

### 2.3 Compiler Wrapper
**責務**: 実際のコンパイラコマンドの実行

**主要機能**:
- コンパイラコマンドの実行
- 標準出力/標準エラーのキャプチャ
- 終了コードの取得
- コンパイラオプションからのソースファイル抽出

**主要クラス**:
```cpp
class CompilerWrapper {
public:
    struct CompilerResult {
        int exit_code;
        std::string stdout_output;
        std::string stderr_output;
        std::vector<std::string> source_files;
    };

    CompilerResult execute(const std::vector<std::string>& command);
    std::vector<std::string> extract_source_files(
        const std::vector<std::string>& command);
};
```

### 2.4 Parser (AST Builder)
**責務**: C++ソースコードの構文解析とAST構築

**戦略**: Clang/LLVM libtoolingを使用
- Clangの強力なパーサーを再利用
- すべてのC++標準に対応
- 正確な型情報とセマンティクス解析

**主要クラス**:
```cpp
class ClangParser {
public:
    std::unique_ptr<ASTContext> parse(
        const std::string& source_file,
        const std::vector<std::string>& compiler_flags);
};

// AST訪問用
class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    bool VisitFunctionDecl(clang::FunctionDecl* decl);
    bool VisitVarDecl(clang::VarDecl* decl);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
    // その他のVisitメソッド
};
```

### 2.5 Rule Engine
**責務**: ルールの管理と実行

**主要機能**:
- ルールの登録
- ルールの有効化/無効化
- ルールの実行
- 診断結果の収集

**主要クラス**:
```cpp
class RuleBase {
public:
    virtual ~RuleBase() = default;
    virtual std::string get_name() const = 0;
    virtual std::vector<Diagnostic> check(const ASTContext& ast) = 0;
    virtual Severity get_severity() const { return Severity::Warning; }
};

class RuleRegistry {
public:
    static RuleRegistry& instance();

    void register_rule(std::unique_ptr<RuleBase> rule);
    void enable_rule(const std::string& name);
    void disable_rule(const std::string& name);
    std::vector<RuleBase*> get_enabled_rules() const;

private:
    std::map<std::string, std::unique_ptr<RuleBase>> rules_;
    std::set<std::string> enabled_rules_;
};

class RuleExecutor {
public:
    std::vector<Diagnostic> execute_rules(
        const ASTContext& ast,
        const std::vector<RuleBase*>& rules);
};
```

### 2.6 Lua Engine
**責須**: Luaスクリプトの実行とC++連携

**主要機能**:
- Luaスクリプトの読み込みと実行
- C++ APIのLuaへの公開
- Luaから報告された診断の収集

**主要クラス**:
```cpp
class LuaEngine {
public:
    LuaEngine();
    ~LuaEngine();

    void load_script(const std::string& script_path);
    void register_api();
    std::vector<Diagnostic> execute_checks(const ASTContext& ast);

private:
    lua_State* L_;
    std::vector<Diagnostic> diagnostics_;
};

// Lua API関数
namespace lua_api {
    int report_error(lua_State* L);
    int report_warning(lua_State* L);
    int get_node_name(lua_State* L);
    int get_node_location(lua_State* L);
    int match_pattern(lua_State* L);
}
```

### 2.7 Diagnostic Reporter
**責務**: 診断結果の収集と整形

**主要クラス**:
```cpp
enum class Severity {
    Error,
    Warning,
    Info
};

struct SourceLocation {
    std::string file;
    int line;
    int column;
};

struct Diagnostic {
    Severity severity;
    std::string rule_name;
    SourceLocation location;
    std::string message;
    std::string source_snippet;
    std::optional<std::string> fix_suggestion;
};

class DiagnosticReporter {
public:
    void add_diagnostic(Diagnostic diag);
    void sort_diagnostics();
    const std::vector<Diagnostic>& get_diagnostics() const;
    int get_error_count() const;
    int get_warning_count() const;

private:
    std::vector<Diagnostic> diagnostics_;
};
```

### 2.8 Output Formatter
**責務**: 診断結果の出力フォーマット

**出力形式**:
- Text: 人間が読みやすい形式
- JSON: ツール連携用
- XML: IDE統合用

**主要クラス**:
```cpp
class OutputFormatter {
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(
        const std::vector<Diagnostic>& diagnostics) = 0;
};

class TextFormatter : public OutputFormatter {
public:
    std::string format(const std::vector<Diagnostic>& diagnostics) override;
};

class JsonFormatter : public OutputFormatter {
public:
    std::string format(const std::vector<Diagnostic>& diagnostics) override;
};

class XmlFormatter : public OutputFormatter {
public:
    std::string format(const std::vector<Diagnostic>& diagnostics) override;
};
```

## 3. データフロー

### 3.1 標準的な実行フロー

```
1. コマンドライン引数解析
   cclint --config=.cclint.yaml g++ -std=c++17 main.cpp
   ↓
2. 設定ファイル読み込み
   .cclint.yaml, custom_rules.lua
   ↓
3. ルール登録
   - YAML定義ルールを登録
   - Luaスクリプトを実行してルールを登録
   ↓
4. コンパイラコマンドからソースファイル抽出
   main.cpp
   ↓
5. 各ソースファイルに対して:
   a. Clangでパース → AST生成
   b. 各ルールを実行
   c. 診断結果を収集
   ↓
6. (オプション) コンパイラ実行
   g++ -std=c++17 main.cpp
   ↓
7. 診断結果を整形して出力
   Text/JSON/XML形式で出力
   ↓
8. 終了コード決定
   エラーがあれば非ゼロ
```

### 3.2 並列処理フロー

```
ソースファイルリスト [file1.cpp, file2.cpp, ..., fileN.cpp]
         ↓
    ┌────┴────┬────────┬────────┐
    ↓         ↓        ↓        ↓
 Thread1   Thread2  Thread3  ThreadN
    │         │        │        │
 Parse     Parse    Parse    Parse
    │         │        │        │
 Execute   Execute  Execute  Execute
 Rules     Rules    Rules    Rules
    │         │        │        │
    └────┬────┴────────┴────────┘
         ↓
    診断結果統合
         ↓
      出力
```

## 4. YAML vs Lua の棲み分け

### 4.1 YAML設定ファイル（cclint.yaml）

**用途**:
- プロジェクト全体の設定
- シンプルなパターンマッチングルール
- ファイルフィルタリング
- 出力形式設定
- ツール動作の制御

**適している場合**:
- 正規表現ベースのチェック（命名規則など）
- 単純な存在チェック（ヘッダーガードなど）
- 設定値の指定（最大行長、インデント幅など）
- 静的なルールセット

**例**:
```yaml
rules:
  - name: function_naming
    pattern: "^[a-z][a-zA-Z0-9]*$"
    target: function_name
    message: "Function names should be camelCase"

  - name: max_line_length
    max: 100
    target: line
    message: "Line exceeds maximum length"

  - name: header_guard
    required: true
    pattern: "^[A-Z_][A-Z0-9_]*_H$"
```

### 4.2 Lua スクリプト（custom_rules.lua）

**用途**:
- 複雑なロジックを持つルール
- コンテキストに依存するチェック
- カスタム計算（サイクロマティック複雑度など）
- ASTの深い解析
- プロジェクト固有の高度なルール

**適している場合**:
- 複数の条件を組み合わせた判定
- データフロー解析
- 型情報を使った検証
- コードメトリクスの計算
- 動的なルール生成

**例**:
```lua
-- 複雑度チェック
function check_cyclomatic_complexity(node)
  if node.type == "FunctionDecl" then
    local complexity = calculate_complexity(node)
    if complexity > 10 then
      report_warning(
        node.location,
        string.format("Function complexity %d exceeds limit 10", complexity)
      )
    end
  end
end

-- プロジェクト固有ルール
function check_error_handling(node)
  if node.type == "CallExpr" and node.name:match("^api_") then
    -- API呼び出しの次の文でエラーチェックがあるか確認
    local next_stmt = get_next_statement(node)
    if not is_error_check(next_stmt) then
      report_error(node.location, "API call must be followed by error check")
    end
  end
end

register_rule("cyclomatic_complexity", check_cyclomatic_complexity)
register_rule("api_error_handling", check_error_handling)
```

### 4.3 選択基準

| 特性 | YAML | Lua |
|-----|------|-----|
| 設定の簡潔さ | ◎ | △ |
| 複雑なロジック | △ | ◎ |
| 保守性（単純ルール） | ◎ | △ |
| 柔軟性 | △ | ◎ |
| パフォーマンス | ◎ | ○ |
| 学習コスト | ◎ | ○ |
| デバッグ容易性 | ○ | △ |

## 5. 依存関係管理

### 5.1 必須依存ライブラリ

1. **LLVM/Clang** (libtooling)
   - バージョン: 14.0以降
   - 用途: C++パース、AST構築
   - リンク: 静的リンク推奨

2. **yaml-cpp**
   - バージョン: 0.7.0以降
   - 用途: YAML設定ファイル解析
   - リンク: 静的/動的両対応

3. **Lua**
   - バージョン: 5.4以降
   - 用途: スクリプトエンジン
   - リンク: 組み込み（ソースコード同梱）

### 5.2 ビルドシステム

**CMake** を使用:
```cmake
cmake_minimum_required(VERSION 3.16)
project(cclint VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(yaml-cpp REQUIRED)

add_subdirectory(third_party/lua)

add_executable(cclint ${SOURCES})
target_link_libraries(cclint
    PRIVATE
    clangTooling
    clangFrontend
    clangASTMatchers
    yaml-cpp
    lua
)
```

## 6. エラーハンドリング戦略

### 6.1 エラーカテゴリ

1. **設定エラー**: 不正な設定ファイル
2. **パースエラー**: C++コードのパースに失敗
3. **ルールエラー**: ルール実行中のエラー
4. **システムエラー**: ファイルI/O、メモリ不足など

### 6.2 エラーハンドリング方針

- 致命的エラー: 即座に終了（exit code 2）
- 回復可能エラー: ログに記録して続行
- ルールエラー: 該当ルールをスキップして続行
- Luaスクリプトエラー: エラー箇所を表示してスキップ

## 7. パフォーマンス最適化

### 7.1 並列処理
- ファイル単位での並列解析（std::thread または OpenMP）
- CPU コア数に応じたスレッド数調整

### 7.2 キャッシュ戦略
- パース結果のキャッシュ（ファイルハッシュベース）
- インクリメンタル解析（変更ファイルのみ再解析）

### 7.3 メモリ管理
- AST の適切な破棄
- ルールごとのメモリ使用量制限

## 8. セキュリティ考慮事項

### 8.1 Lua サンドボックス
- ファイルI/O制限（設定で許可されたパスのみ）
- ネットワークアクセス禁止
- システムコマンド実行禁止
- メモリ使用量制限

### 8.2 設定ファイル検証
- YAML構造の検証
- パス トラバーサル防止
- 環境変数インジェクション防止

## 9. 拡張性設計

### 9.1 プラグインシステム（将来）
- 共有ライブラリ (.so, .dylib) としてのルール
- プラグイン API の定義
- 動的ロード機構

### 9.2 カスタム出力フォーマット
- OutputFormatter の継承
- プラグインとして追加可能

### 9.3 IDE 統合
- LSP サーバーモード
- JSON-RPC 通信
- リアルタイム解析
