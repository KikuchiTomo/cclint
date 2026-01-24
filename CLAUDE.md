# Claude Code 開発ガイド

このドキュメントは、Claude Codeを使用してcclintプロジェクトを開発する際のガイドラインです。

## プロジェクト概要

**プロジェクト名**: cclint - Customizable C++ Linter
**開発言語**: C++17
**対象環境**: Ubuntu 20.04+, macOS 11+
**開発体制**: Claude Code支援による開発

## 重要な原則

### 1. TODOリストの管理（最重要）

**docs/TODO.md** はプロジェクトの進捗管理の中心です。すべての作業はTODOリストに基づいて行います。
同時に複数作業したい場合はブランチとworktreeを活用しましょう．またちょっとした作業や調査，開発にはsubagentを使用してください

#### 作業開始前の必須手順

```
1. docs/TODO.mdを開いて確認する
2. 作業するタスクを見つける
3. タスクのステータスを [ ] から [>] に変更する
4. 依存関係を確認する
5. 作業を開始する
```

#### 作業中の手順

```
1. 新しいタスクが見つかったらTODO.mdに追加する
2. ブロック事項があれば [!] にして理由を記載する
3. 進捗をこまめにTODO.mdに反映する
4. 関連するドキュメントも併せて更新する
```

#### 作業完了後の必須手順

```
1. タスクのステータスを [>] から [x] に変更する
2. 実装したコードをテストする
3. 関連ドキュメントを更新する
4. 変更をコミットする
5. TODO.mdの変更もコミットに含める
```

#### TODOステータスの意味

- `[ ]` 未着手
- `[>]` 作業中（必ず1つのタスクのみ作業中にする）
- `[x]` 完了
- `[!]` ブロック中（依存関係待ち、問題発生など）
- `[~]` 保留

### 2. ドキュメント駆動開発

実装前に必ずドキュメントを確認してください：

1. **requirements.md**: 何を作るべきか
2. **design.md**: どう設計するか
3. **detailed_design.md**: 具体的にどう実装するか
4. **TODO.md**: 今何をすべきか

実装後にドキュメントと実装が乖離した場合は、ドキュメントを更新してください。

### 3. テスト駆動開発（推奨）

新しい機能を実装する際は：

1. テストを先に書く（tests/以下）
2. テストが失敗することを確認する
3. 実装する
4. テストが通ることを確認する
5. リファクタリングする

## ディレクトリ構造

```
cclint/
├── docs/                       # ドキュメント
│   ├── requirements.md         # 要件定義
│   ├── design.md               # 設計書
│   ├── detailed_design.md      # 詳細設計
│   ├── milestones.md           # マイルストーン
│   └── TODO.md                 # TODOリスト（最重要！）
├── src/                        # ソースコード
│   ├── main.cpp                # エントリーポイント
│   ├── cli/                    # コマンドライン解析
│   ├── config/                 # 設定ファイル処理
│   ├── compiler/               # コンパイララッパー
│   ├── parser/                 # C++パーサー（Clang統合）
│   ├── rules/                  # ルールシステム
│   │   └── builtin/            # 組み込みルール
│   ├── lua/                    # LuaJIT統合
│   ├── engine/                 # 解析エンジン
│   ├── diagnostic/             # 診断レポート
│   ├── output/                 # 出力フォーマッター
│   └── util/                   # ユーティリティ
├── tests/                      # テストコード
│   ├── unit/                   # 単体テスト
│   ├── integration/            # 統合テスト
│   └── samples/                # テスト用サンプルコード
├── examples/                   # サンプル・例
│   ├── configs/                # YAML設定例
│   ├── rules/                  # Luaルール例
│   └── ci/                     # CI/CD統合例
├── third_party/                # サードパーティライブラリ
│   └── luajit/                 # LuaJIT（サブモジュール）
├── CMakeLists.txt              # ビルド設定
├── README.md                   # プロジェクト概要
└── CLAUDE.md                   # このファイル
```

## コーディング規約

### C++コーディングスタイル

#### 命名規則

```cpp
// クラス名: PascalCase
class ArgumentParser { };

// 関数名: snake_case
void parse_arguments() { }

// 変数名: snake_case
int file_count = 0;

// 定数: UPPER_CASE
const int MAX_ERRORS = 100;

// メンバ変数: 末尾にアンダースコア
class Config {
private:
    std::string version_;
    int max_errors_;
};

// 名前空間: snake_case
namespace config {
namespace detail {
}
}
```

#### ファイル構成

```cpp
// example.hpp
#pragma once

#include <standard_library>  // 標準ライブラリ

#include <third_party>       // サードパーティ

#include "project_header.hpp" // プロジェクト内ヘッダー

namespace cclint {
namespace module_name {

class ClassName {
public:
    // パブリックメソッド

private:
    // プライベートメソッド
    // メンバ変数
};

} // namespace module_name
} // namespace cclint
```

#### インクルードガード

```cpp
// 常に #pragma once を使用
#pragma once

// 従来の #ifndef/#define は使わない
```

#### コメント

```cpp
// 良いコメント: なぜそうするかを説明
// Parse the configuration file to extract rule definitions
// We use yaml-cpp here because it provides better error messages
// than other YAML parsers.
auto config = yaml_parser.parse(file_path);

// 悪いコメント: コードを繰り返すだけ
// Parse the config file
auto config = yaml_parser.parse(file_path);

// 関数ドキュメント
/// Parses a C++ source file and builds an AST.
///
/// @param source_file Path to the source file to parse
/// @param compiler_flags Compiler flags to use during parsing
/// @return Unique pointer to the parsed ASTUnit
/// @throws std::runtime_error if parsing fails
std::unique_ptr<clang::ASTUnit> parse(
    const std::string& source_file,
    const std::vector<std::string>& compiler_flags);
```

### モダンC++の使用

#### スマートポインタ

```cpp
// 生ポインタは使わない
Rule* rule = new Rule();  // ❌

// スマートポインタを使う
auto rule = std::make_unique<Rule>();  // ✅

// 所有権を移譲する場合
registry.register_rule(std::move(rule));  // ✅
```

#### 範囲ベースfor

```cpp
// イテレータは使わない
for (auto it = vec.begin(); it != vec.end(); ++it) {  // ❌
    process(*it);
}

// 範囲ベースforを使う
for (const auto& item : vec) {  // ✅
    process(item);
}
```

#### auto の使用

```cpp
// 型が明白な場合は auto を使う
auto parser = std::make_unique<ClangParser>();  // ✅
auto count = diagnostics.size();  // ✅

// 型が不明瞭な場合は明示的に
Config config = load_config();  // ✅（Configが何かわかる）
```

#### const の使用

```cpp
// 変更しない変数は const
const auto max_errors = config.max_errors;

// 変更しない参照は const &
void process(const std::vector<Diagnostic>& diagnostics);

// 変更する参照は &
void modify(std::vector<Diagnostic>& diagnostics);

// 移動する場合は &&
void take_ownership(std::unique_ptr<Rule>&& rule);
```

## ビルドとテスト

### ビルド

```bash
# デバッグビルド
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# リリースビルド
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### テストの実行

```bash
# すべてのテストを実行
cd build
ctest

# 特定のテストを実行
./tests/cli_test

# テストを詳細モードで実行
ctest --verbose
```

### 静的解析

```bash
# clang-tidy
clang-tidy src/**/*.cpp -- -std=c++17 -I include

# cppcheck
cppcheck --enable=all --std=c++17 src/
```

## Git ワークフロー

### ブランチ戦略

- `main`: 安定版
- `develop`: 開発中
- `feature/feature-name`: 機能開発
- `fix/bug-description`: バグ修正

### コミットメッセージ

```
[マイルストーン番号] 簡潔なタイトル（50文字以内）

詳細な説明（必要に応じて）
- 何を変更したか
- なぜ変更したか
- 影響範囲

Closes #issue番号
```

例：
```
[M1] Implement ArgumentParser for CLI

- Added ArgumentParser class to parse command-line arguments
- Implemented support for --config, --format, --verbose options
- Added help message generation
- Updated TODO.md to mark tasks as completed

Closes #3
```

### コミット前のチェックリスト

- [ ] コードがビルドできる
- [ ] 新しいテストを追加した（機能追加の場合）
- [ ] すべてのテストが通る
- [ ] TODO.mdを更新した
- [ ] 関連ドキュメントを更新した
- [ ] コードをフォーマットした（clang-format）
- [ ] 静的解析を実行した（警告がない）

## デバッグ

### デバッグビルド

```bash
# デバッグシンボル付きでビルド
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# gdbで実行
gdb ./cclint
(gdb) run --config=test.yaml g++ test.cpp
```

### ログ出力

```cpp
#include "util/logger.hpp"

// ログレベル: ERROR, WARNING, INFO, DEBUG
LOG_DEBUG("Parsing file: " << file_path);
LOG_INFO("Found " << count << " diagnostics");
LOG_WARNING("Configuration file not found, using defaults");
LOG_ERROR("Failed to parse: " << error_message);
```

### メモリリーク検査

```bash
# valgrindでメモリリークをチェック
valgrind --leak-check=full --show-leak-kinds=all ./cclint g++ test.cpp
```

## パフォーマンス最適化

### プロファイリング

```bash
# gperfでプロファイル
g++ -pg -O2 ...
./cclint g++ test.cpp
gprof ./cclint gmon.out > profile.txt

# perfでプロファイル（Linux）
perf record ./cclint g++ test.cpp
perf report
```

### 最適化のヒント

1. **不要なコピーを避ける**: move semanticsを使う
2. **const参照を使う**: 大きなオブジェクトは参照渡し
3. **並列処理**: 独立したファイルは並列解析
4. **キャッシュ**: パース結果をキャッシュ
5. **早期リターン**: 不要な処理はスキップ

## トラブルシューティング

### ビルドエラー

#### LLVM/Clangが見つからない

```bash
# Ubuntu
sudo apt install llvm-14-dev libclang-14-dev

# macOS
brew install llvm

# CMakeにパスを指定
cmake -DLLVM_DIR=/usr/lib/llvm-14/cmake ..
```

#### yaml-cppが見つからない

```bash
# Ubuntu
sudo apt install libyaml-cpp-dev

# macOS
brew install yaml-cpp
```

### ランタイムエラー

#### セグメンテーションフォルト

```bash
# gdbでバックトレースを取得
gdb ./cclint
(gdb) run --config=test.yaml g++ test.cpp
# クラッシュ後
(gdb) backtrace
```

#### LuaJITエラー

```bash
# Luaスクリプトのデバッグ
cclint --verbose --lua=debug_rule.lua g++ test.cpp
```

## マイルストーンごとの重点事項

### Milestone 1: MVP
- **重点**: 基本的な動作を実現する
- **注意**: 完璧を目指さず、動くものを作る
- **テスト**: 手動テストで十分

### Milestone 2: ルールシステム
- **重点**: 拡張可能な設計
- **注意**: ルール追加が簡単にできるようにする
- **テスト**: ルールごとに単体テスト必須

### Milestone 3: LuaJIT統合
- **重点**: セキュリティとパフォーマンス
- **注意**: サンドボックスを厳格に
- **テスト**: Luaエラーハンドリングを念入りに

### Milestone 4: パフォーマンス
- **重点**: 測定可能な高速化
- **注意**: 最適化前にプロファイル
- **テスト**: ベンチマークを作成

### Milestone 5: ツール統合
- **重点**: 他ツールとの連携
- **注意**: 標準的なフォーマットに従う
- **テスト**: CI環境で実際に動作確認

### Milestone 6: リリース準備
- **重点**: 品質とドキュメント
- **注意**: ユーザー目線で考える
- **テスト**: すべてのプラットフォームで確認

## よくある質問（FAQ）

### Q: 新しい機能を追加したい場合は？

1. TODO.mdに新しいタスクを追加
2. 必要に応じてdesign.mdを更新
3. テストを書く
4. 実装する
5. TODO.mdを更新

### Q: バグを見つけた場合は？

1. 再現手順を確認
2. テストケースを追加（バグを再現するテスト）
3. 修正する
4. テストが通ることを確認
5. TODO.mdに記録

### Q: ドキュメントと実装が乖離している場合は？

1. 実装が正しければドキュメントを更新
2. ドキュメントが正しければ実装を修正
3. 両方見直す必要があれば議論

### Q: パフォーマンスが悪い場合は？

1. プロファイルを取る
2. ボトルネックを特定
3. 最適化する
4. 効果を測定
5. 改善されていなければ戻す

## 参考資料

### C++
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Modern C++ Features](https://github.com/AnthonyCalandra/modern-cpp-features)

### Clang/LLVM
- [Clang LibTooling](https://clang.llvm.org/docs/LibTooling.html)
- [Clang AST Matcher Reference](https://clang.llvm.org/docs/LibASTMatchersReference.html)

### LuaJIT
- [LuaJIT Documentation](https://luajit.org/luajit.html)
- [Lua 5.1 Reference Manual](https://www.lua.org/manual/5.1/)
- [LuaJIT C API](https://luajit.org/ext_c_api.html)

### YAML
- [yaml-cpp Tutorial](https://github.com/jbeder/yaml-cpp/wiki/Tutorial)
- [YAML Specification](https://yaml.org/spec/)

## 長時間作業モード

### 指示内容（2026-01-23追加）

ユーザーが不在の間、token使い切るまで自律的に作業を続ける場合の手順：

1. **PRを分けない**: 適宜commitしながら同じブランチで作業を続ける
2. **TODOリストに従う**: docs/TODO.mdにあるタスクを一つずつ実行
3. **途中で止めない**: token使い切るまでずっと作業を続ける
4. **適宜commit**: 意味のある単位でcommitする（PRは後でまとめて作成）
5. **承認不要**: ユーザーの承認を待たずに進める

### 作業の進め方

```
1. docs/TODO.mdを読む
2. 未着手のタスクを見つける
3. タスクを [>] に変更
4. タスクを実装
5. テストする
6. commitする
7. タスクを [x] に変更
8. TODO.mdもcommitする
9. 次のタスクへ（1に戻る）
10. token切れまで繰り返す
```

### commitメッセージの形式

```
<type>: <タスクの簡潔な説明>

実装内容の詳細:
- 具体的な変更1
- 具体的な変更2
- 具体的な変更3

関連TODO項目: [Milestone X]

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

### 注意事項

- 一つのタスクが複雑な場合は、さらに小さいタスクに分割してからcommit
- ビルドエラーが出た場合は、必ず修正してからcommit
- テストは可能な限り実行してから commit
- ドキュメント更新も忘れずに
- 会話がコンパクトされる可能性があるため、この指示内容をCLAUDE.mdに記録

### 優先順位

1. Milestone 1のタスク（MVP: 最小動作版）
2. Milestone 2のタスク（ルールシステム基盤）
3. ドキュメント更新
4. テスト作成
5. その他の改善

### 最新の作業指示（2026-01-23 22:01追加）

ユーザーからの指示:
```
PRを分けなくて構いませんので，適宜commitしならがらTODOにあることを進めてください．
私は寝ますので，夜中の間token使い切るまでずっと作業お願いしますね．
途中で作業をやめないでください．
commitしてそのままでいいですから，TODOにあることを一つ一つ実行してください．
会話がコンパクトした時に，困ると思うので，この内容をclaude.mdに追記してから作業を開始してください．
承認は必要ありません
```

実行内容:
- TODOリストのタスクを順番に実装
- 各タスク完了後に適宜commit
- token使い切るまで継続（途中で止めない）
- ユーザーの承認を待たずに自律的に進める
- PRはまとめて後で作成（個別に分けない）

## 最後に

このプロジェクトの成功の鍵は **TODO.mdの適切な管理** です。

作業を始める前に必ずTODO.mdを確認し、作業が完了したら必ず更新してください。

これにより：
- 進捗が可視化される
- 何をすべきかが明確になる
- 作業の重複を防げる
- プロジェクトの方向性が保たれる

Happy Coding! 🚀
