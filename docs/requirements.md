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

# ルールの定義と適用順序
rules:
  # シンプルな指定（デフォルト設定で有効化）
  - naming_conventions
  - header_guards

  # 詳細な設定（優先度、severity、パラメータを指定）
  - name: max_line_length
    enabled: true
    priority: 10  # 小さい数値ほど先に実行
    severity: warning
    parameters:
      max: 100

  - name: brace_style
    enabled: true
    priority: 20
    severity: info
    parameters:
      style: K&R

# Luaスクリプトの読み込み（rulesより後に実行される）
lua_scripts:
  - path: ".cclint/custom_rules.lua"
    priority: 100  # YAML定義ルールの後に実行
  - path: ".cclint/project_specific.lua"
    priority: 200  # さらに後に実行

# ルール実行設定
rule_execution:
  # ルールの並列実行を許可するか（依存関係がない場合のみ）
  parallel: true
  # 1つのルールが失敗したら即座に停止するか
  fail_fast: false

output: text
max_errors: 100
compiler:
  pass_through: true
  show_compiler_output: true
```

#### 2.2.2 Lua設定スクリプト
用途: 複雑なロジックを持つルール、動的なルール定義

**重要な設計原則**: Luaスクリプト内でユーザーが自由にロジックを記述できるように、cclintは**大量のAPI**を提供する。単なるパターンマッチングではなく、制御フロー解析、スタイルチェック、命名規則など、あらゆる静的解析をLua側で実装可能にする。

機能:
- 完全なASTへのアクセスと走査
- 制御フロー解析API
- データフロー解析API
- スタイル・フォーマット情報へのアクセス
- ファイル・プロジェクト情報へのアクセス
- カスタムチェックロジックの実装
- 正規表現パターンマッチング
- 条件付きルール適用

##### 対応すべき詳細なチェック項目

1. **ファイル名とクラス名の関係**
   - 特定のファイル名に対するクラス名の規則
   - クラス名とファイル名の一致チェック
   - 複数クラスが同一ファイルにある場合の検出

2. **アクセス指定子別の命名規則**
   - `private`メンバ関数の命名規則
   - `public`メンバ関数の命名規則
   - `protected`メンバ関数の命名規則
   - アクセス指定子ごとに異なるルールを適用可能

3. **コードスタイルチェック**
   - `{}`の位置（K&R, Allman, GNU, etc.）
   - `if`文での`{}`必須化
   - インデント深度
   - 行の長さ
   - スペース・タブの使用

4. **clang-tidy相当の全チェック項目**
   - modernize系チェック（auto, nullptr, etc.）
   - performance系チェック
   - readability系チェック
   - bugprone系チェック
   - すべてのチェックをLuaで再実装可能

5. **命名規則の詳細**
   - マクロ命名規則（例: `UPPER_CASE`）
   - static変数命名規則
   - グローバル変数命名規則（例: `g_prefix`）
   - クラスメンバ変数命名規則（例: `trailing_`）
   - static クラスメンバ変数命名規則（例: `s_prefix`）
   - static メソッド命名規則
   - const/constexpr変数命名規則

6. **制御フローチェック**
   - `switch`文での`break`必須化
   - fall-through の検出
   - 到達不可能コードの検出
   - 無限ループの検出

7. **スタイル・フォーマットの柔軟な制御**
   - 特定の変数名の後の空行数チェック
   - 特定のクラス名の場合の特別なルール適用
   - セクションコメント前後の空行数
   - 関数間の空行数
   - include文のグループ化と空行

8. **プロジェクト固有のルール**
   - 特定のクラス名やファイル名に対する任意のルール
   - プロジェクト内の慣習のチェック
   - 命名規則の例外処理（特定のパターンのみ許可など）

Lua API例:
```lua
-- 例1: ファイル名とクラス名の対応チェック
function check_class_filename_match(node)
  if node.type == "CXXRecordDecl" and not node.is_implicit then
    local class_name = node.name
    local filename = get_filename(node.location)
    local basename = filename:match("([^/]+)%.%w+$")

    if class_name ~= basename then
      report_warning(
        node.location,
        string.format("Class name '%s' should match filename '%s'",
                      class_name, basename)
      )
    end
  end
end

-- 例2: アクセス指定子別の命名規則
function check_member_function_naming(node)
  if node.type == "CXXMethodDecl" then
    local name = node.name
    local access = get_access_specifier(node)

    if access == "private" then
      -- privateメソッドはアンダースコアで始まる
      if not name:match("^_") then
        report_error(node.location,
          "Private methods should start with underscore")
      end
    elseif access == "public" then
      -- publicメソッドはcamelCase
      if not name:match("^[a-z][a-zA-Z0-9]*$") then
        report_error(node.location,
          "Public methods should be camelCase")
      end
    end
  end
end

-- 例3: if文での{}必須化
function check_if_braces(node)
  if node.type == "IfStmt" then
    local then_stmt = get_then_statement(node)
    if then_stmt.type ~= "CompoundStmt" then
      report_error(node.location,
        "If statement must use braces")
    end

    -- elseがある場合もチェック
    if has_else(node) then
      local else_stmt = get_else_statement(node)
      if else_stmt.type ~= "CompoundStmt" and
         else_stmt.type ~= "IfStmt" then
        report_error(get_else_location(node),
          "Else statement must use braces")
      end
    end
  end
end

-- 例4: switch文のbreak必須化（fall-throughの検出）
function check_switch_fallthrough(node)
  if node.type == "SwitchStmt" then
    local cases = get_case_statements(node)
    for i, case_stmt in ipairs(cases) do
      if i < #cases then  -- 最後のcase以外
        local body = get_case_body(case_stmt)
        if not has_break_or_return(body) and not has_fallthrough_comment(body) then
          report_warning(case_stmt.location,
            "Case statement should end with break or return, " ..
            "or have [[fallthrough]] comment")
        end
      end
    end
  end
end

-- 例5: static変数の命名規則
function check_static_variable_naming(node)
  if node.type == "VarDecl" then
    local name = node.name
    local storage = get_storage_class(node)
    local context = get_declaration_context(node)

    if storage == "static" then
      if context == "class" then
        -- クラスのstaticメンバ変数は s_ で始まる
        if not name:match("^s_") then
          report_error(node.location,
            "Static member variables should start with 's_'")
        end
      elseif context == "global" or context == "namespace" then
        -- グローバルstatic変数は g_ で始まる
        if not name:match("^g_") then
          report_error(node.location,
            "Global static variables should start with 'g_'")
        end
      end
    end
  end
end

-- 例6: {}の位置チェック（K&R スタイル）
function check_brace_style(node)
  if node.type == "FunctionDecl" and has_body(node) then
    local func_end_loc = get_declaration_end_location(node)
    local body_start_loc = get_body_start_location(node)

    -- 関数宣言の最後と{が同じ行にあるかチェック
    if func_end_loc.line ~= body_start_loc.line then
      report_warning(body_start_loc,
        "Opening brace should be on the same line as function declaration (K&R style)")
    end
  end
end

-- 例7: マクロ命名規則
function check_macro_naming(node)
  if node.type == "MacroDefinition" then
    local name = node.name
    -- マクロは全て大文字でアンダースコア区切り
    if not name:match("^[A-Z][A-Z0-9_]*$") then
      report_error(node.location,
        "Macro names should be UPPER_CASE")
    end
  end
end

-- 例8: 特定の変数名の後の空行数チェック
function check_spacing_after_variable(node)
  if node.type == "VarDecl" then
    local name = node.name
    -- "s_instance" という名前のstatic変数の後は2行空ける
    if name == "s_instance" then
      local next_node = get_next_sibling(node)
      if next_node then
        local blank_lines = count_blank_lines_between(node, next_node)
        if blank_lines < 2 then
          report_warning(node.location,
            string.format("Expected 2 blank lines after '%s', found %d",
                          name, blank_lines))
        end
      end
    end
  end
end

-- 例9: 特定のクラス名の場合の特別なルール
function check_special_class_rules(node)
  if node.type == "CXXRecordDecl" then
    local class_name = node.name

    -- "Handler"で終わるクラスには特別なルール
    if class_name:match("Handler$") then
      -- 必ずprocess()メソッドを持つ必要がある
      local methods = get_methods(node)
      local has_process = false
      for _, method in ipairs(methods) do
        if method.name == "process" then
          has_process = true
          break
        end
      end

      if not has_process then
        report_error(node.location,
          "Handler classes must have a 'process()' method")
      end
    end

    -- "Singleton"で終わるクラスには特別なルール
    if class_name:match("Singleton$") then
      -- getInstance() メソッドが必須
      -- コンストラクタがprivateである必要がある
      local has_get_instance = false
      local has_private_ctor = false

      local methods = get_methods(node)
      for _, method in ipairs(methods) do
        if method.name == "getInstance" and is_static_method(method) then
          has_get_instance = true
        end
        if method.is_constructor and get_access_specifier(method) == "private" then
          has_private_ctor = true
        end
      end

      if not has_get_instance then
        report_error(node.location,
          "Singleton classes must have a static getInstance() method")
      end
      if not has_private_ctor then
        report_error(node.location,
          "Singleton classes must have a private constructor")
      end
    end
  end
end

-- 例10: セクションコメント前後の空行
function check_section_comment_spacing(comments)
  for _, comment in ipairs(comments) do
    local text = get_comment_text(comment)
    -- セクションコメント（複数行で区切り線がある）の場合
    if text:match("^//=+") or text:match("^//%*+") then
      local loc = comment.location
      -- コメントの前に1行以上の空行が必要
      local blank_before = count_blank_lines_before(loc)
      if blank_before < 1 then
        report_warning(loc,
          "Section comment should be preceded by at least 1 blank line")
      end
    end
  end
end

-- 例11: 関数間の空行数チェック
function check_function_spacing(node)
  if node.type == "FunctionDecl" then
    local prev_func = find_previous_function(node)
    if prev_func then
      local blank_lines = count_blank_lines_between(prev_func, node)
      -- 関数と関数の間は2行空ける
      if blank_lines < 2 then
        report_warning(node.location,
          string.format("Expected 2 blank lines between functions, found %d",
                        blank_lines))
      end
    end
  end
end

-- ルール登録
register_rule("class_filename_match", check_class_filename_match)
register_rule("member_function_naming", check_member_function_naming)
register_rule("if_braces", check_if_braces)
register_rule("switch_fallthrough", check_switch_fallthrough)
register_rule("static_variable_naming", check_static_variable_naming)
register_rule("brace_style", check_brace_style)
register_rule("macro_naming", check_macro_naming)
register_rule("spacing_after_variable", check_spacing_after_variable)
register_rule("special_class_rules", check_special_class_rules)
register_rule("section_comment_spacing", check_section_comment_spacing)
register_rule("function_spacing", check_function_spacing)
```

#### 2.2.3 Lua API仕様

cclintはLuaスクリプトから呼び出せる大量のAPIを提供します。これにより、ユーザーは自由にチェックロジックを実装できます。

##### 診断報告API
```lua
-- エラー報告
report_error(location, message)

-- 警告報告
report_warning(location, message)

-- 情報報告
report_info(location, message)

-- 修正提案付き報告
report_with_fix(severity, location, message, fix_text)
```

##### ASTノード操作API
```lua
-- ノード型の取得
node.type  -- 文字列: "FunctionDecl", "VarDecl", etc.

-- ノード名の取得
node.name  -- 宣言の名前

-- ノードの位置情報
node.location  -- {file, line, column}

-- 暗黙的な宣言かどうか
node.is_implicit  -- boolean

-- ノードの子要素を取得
get_children(node)  -- ノードの配列

-- 親ノードを取得
get_parent(node)

-- 次の兄弟ノードを取得
get_next_sibling(node)

-- 前の兄弟ノードを取得
get_prev_sibling(node)
```

##### 関数・メソッド関連API
```lua
-- 関数が定義を持つか
has_body(node)

-- 関数の本体を取得
get_function_body(node)

-- 関数のパラメータを取得
get_parameters(node)

-- 関数の戻り値の型を取得
get_return_type(node)

-- メソッドのアクセス指定子を取得
get_access_specifier(node)  -- "public", "private", "protected"

-- メソッドがstaticか
is_static_method(node)

-- メソッドがvirtualか
is_virtual_method(node)

-- メソッドがconstか
is_const_method(node)
```

##### 変数・フィールド関連API
```lua
-- 変数のストレージクラスを取得
get_storage_class(node)  -- "static", "extern", "auto", etc.

-- 変数の型を取得
get_variable_type(node)

-- 変数が定数か
is_const_variable(node)

-- 変数がconstexprか
is_constexpr_variable(node)

-- 変数の初期化式を取得
get_initializer(node)

-- クラスメンバ変数のアクセス指定子を取得
get_member_access(node)  -- "public", "private", "protected"
```

##### クラス関連API
```lua
-- クラスの基底クラスを取得
get_base_classes(node)

-- クラスのメンバを取得
get_class_members(node)

-- クラスがabstractか
is_abstract_class(node)

-- クラスがtemplateか
is_template_class(node)

-- クラスの全メソッドを取得
get_methods(node)

-- クラスの全フィールドを取得
get_fields(node)
```

##### 制御フロー関連API
```lua
-- if文のthen節を取得
get_then_statement(node)

-- if文がelse節を持つか
has_else(node)

-- if文のelse節を取得
get_else_statement(node)

-- switch文のcase文を取得
get_case_statements(node)

-- case文の本体を取得
get_case_body(node)

-- 文がbreakを含むか
has_break(node)

-- 文がreturnを含むか
has_return(node)

-- 文がbreak または return を含むか
has_break_or_return(node)

-- fall-throughコメントがあるか
has_fallthrough_comment(node)

-- ループの本体を取得
get_loop_body(node)

-- ループ条件を取得
get_loop_condition(node)
```

##### マクロ関連API
```lua
-- マクロ定義の名前を取得
get_macro_name(node)

-- マクロ定義の本体を取得
get_macro_body(node)

-- マクロが関数形式か
is_function_macro(node)

-- マクロのパラメータを取得
get_macro_parameters(node)
```

##### ファイル・位置情報API
```lua
-- ファイル名を取得
get_filename(location)

-- ファイルの絶対パスを取得
get_absolute_path(location)

-- 行番号を取得
location.line

-- 列番号を取得
location.column

-- 宣言の終了位置を取得
get_declaration_end_location(node)

-- 本体の開始位置を取得
get_body_start_location(node)

-- ソースコードの範囲を取得
get_source_range(node)

-- ソースコードのテキストを取得
get_source_text(node)
```

##### スタイル・フォーマット情報API
```lua
-- インデント幅を取得
get_indentation(location)

-- 行の長さを取得
get_line_length(location)

-- 行のテキストを取得
get_line_text(location)

-- 空白がタブかスペースか
uses_tabs(location)

-- {の位置を取得
get_brace_location(node)

-- }の位置を取得
get_closing_brace_location(node)
```

##### 宣言コンテキストAPI
```lua
-- 宣言のコンテキストを取得
get_declaration_context(node)  -- "class", "namespace", "function", "global"

-- 現在の名前空間を取得
get_current_namespace(node)

-- 現在のクラスを取得
get_current_class(node)

-- 現在の関数を取得
get_current_function(node)
```

##### 型情報API
```lua
-- 型名を取得
get_type_name(type)

-- 型がポインタか
is_pointer_type(type)

-- 型が参照か
is_reference_type(type)

-- 型がconstか
is_const_type(type)

-- 型がビルトイン型か
is_builtin_type(type)

-- 型がクラス型か
is_class_type(type)
```

##### コメント関連API
```lua
-- ノードに関連するコメントを取得
get_comments(node)

-- ファイル内の全コメントを取得
get_all_comments(file)

-- コメントのテキストを取得
get_comment_text(comment)

-- Doxygenコメントがあるか
has_doxygen_comment(node)

-- コメントの種類を取得
get_comment_kind(comment)  -- "line", "block", "doxygen"

-- コメントの位置を取得
comment.location
```

##### 空行・スペーシング関連API
```lua
-- ノード間の空行数をカウント
count_blank_lines_between(node1, node2)

-- 位置の前の空行数をカウント
count_blank_lines_before(location)

-- 位置の後の空行数をカウント
count_blank_lines_after(location)

-- 2つの位置間の行数を取得
get_line_count_between(location1, location2)

-- 行が空行かどうか
is_blank_line(location)

-- 行が空白のみか
is_whitespace_only_line(location)
```

##### 検索・走査API
```lua
-- 前の関数を検索
find_previous_function(node)

-- 次の関数を検索
find_next_function(node)

-- 前の宣言を検索
find_previous_declaration(node)

-- 特定の型の前のノードを検索
find_previous_node_of_type(node, type)

-- 特定の型の次のノードを検索
find_next_node_of_type(node, type)

-- 条件に一致する最初のノードを検索
find_first(root, predicate)

-- 条件に一致するすべてのノードを検索
find_all(root, predicate)
```

##### ユーティリティAPI
```lua
-- 正規表現マッチング
string.match(text, pattern)

-- ASTを再帰的に走査
traverse_ast(root, callback)

-- 特定の型のノードを検索
find_nodes_of_type(root, node_type)

-- 名前でノードを検索
find_nodes_by_name(root, name)
```

#### 2.2.4 設定ファイルの優先順位
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
- **LSP（Language Server Protocol）サーバー実装**
  - リアルタイムでコードをチェック
  - エディタ内で診断結果を表示
  - コード編集中に即座にフィードバック
  - Quick Fix提案の表示
- **IDE統合（優先順位順）**
  - VSCode拡張機能
    - 診断結果のインライン表示
    - Problems パネルへの統合
    - Quick Fix actions
    - ルール設定UI
  - Vim/Neovim プラグイン
    - ALE統合
    - Quickfix list連携
    - 非同期チェック
  - CLion プラグイン
  - Visual Studio拡張
- クラウドベースのルール共有プラットフォーム
  - コミュニティルールの共有
  - ルールの検索とインストール
  - バージョン管理

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
