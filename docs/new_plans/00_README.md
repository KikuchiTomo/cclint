# Lua完全移行プロジェクト - 全体概要

**作成日**: 2026-01-28
**プロジェクト名**: 組み込みルール削除とLua完全移行

## プロジェクトの目的

cclintを「組み込みルールなし、Lua完全カスタマイズ」というコンセプトに移行する。
- 組み込みC++ルールを全て削除
- Luaスクリプトで全てのルールを定義できるようにする
- より強力で直感的なLua APIを提供する

## コンセプト

cclintは「Luaで独自にルールを定義できる」ことが最大の特徴：
- 組み込みルールは一切提供しない（ゼロから自由に定義）
- Luaスクリプトで簡単にカスタムルールを作成
- C++のあらゆる要素にアクセス可能
- メソッドチェーンで直感的にコードを探索・検証

## ドキュメント構成

### このディレクトリのファイル

1. **00_README.md** (このファイル) - 全体概要
2. **01_TODO.md** - 詳細なTODOリスト（9フェーズ）
3. **02_IMPACT_ANALYSIS.md** - 影響範囲分析とリスク評価
4. **03_API_VISION.md** - 実現したいLua APIの具体例
5. **04_IMPLEMENTATION_GUIDE.md** - 実装の優先順位と進め方
6. **05_API_SPECIFICATION.md** - 新しいLua API仕様
7. **06_CODE_EXAMPLES.md** - 実装時のコード例とスニペット
8. **07_PROGRESS.md** - 進捗管理（作業開始時に作成）

## 主要な変更

### 削除されるもの
- `src/rules/builtin/` 全体（4ファイル）
  - max_line_length.cpp/.hpp
  - header_guard.cpp/.hpp
  - function_complexity.cpp/.hpp
  - naming_convention.cpp/.hpp

### 追加されるもの
- 新しいASTノード（IfStatement, SwitchStatement, LoopStatement, TryStatement, Macro, CallExpression）
- コールグラフビルダー（CallGraphBuilder）
- 20以上の新しいLua API
- メソッドチェーン機能
- インデント情報API

### 変更されるもの
- `src/parser/ast.hpp` - ASTノード定義拡張
- `src/parser/simple_parser.cpp` - パーサー機能拡張
- `src/lua/lua_bridge.cpp` - Lua API大幅拡張
- `src/config/yaml_config.cpp` - 設定形式変更
- ドキュメント全般

## 実装の流れ

### Phase 1: 準備（1-2日）
- ASTノード拡張設計の詳細化
- Lua API設計の詳細化

### Phase 2: ASTパーサー拡張（3-4日）
- 制御構造のパース（if, switch, for, while, try）
- マクロ定義のパース
- インデント情報の記録
- コールグラフ構築

### Phase 3: Lua API実装（4-5日）
- 基本取得API（get_files, get_functions, get_mains, get_enums, get_ifs, etc.）
- 関係性API（callers, callees）
- インデント・スタイルAPI
- メソッドチェーン機能

### Phase 4: 組み込みルール削除（1日）
- 組み込みルールのLua版作成
- C++実装の削除
- ビルドシステム更新

### Phase 5: 設定とドキュメント（2-3日）
- YAML設定形式更新
- README.md更新
- Lua API Reference更新
- サンプル作成

### Phase 6: テストと最適化（2-3日）
- 新しいAPIのテスト
- 統合テスト
- パフォーマンス最適化

**合計**: 約15-20日（MVP）

## クイックスタート（実装開始時）

### 1. 現在の状態確認
```bash
cd /Users/tomokikuchi/repos/cclint
git status
```

### 2. 新しいブランチ作成
```bash
git checkout -b feature/lua-full-migration
```

### 3. ドキュメント確認
- `01_TODO.md` - やるべきことの全体像
- `04_IMPLEMENTATION_GUIDE.md` - 具体的な実装手順
- `05_API_SPECIFICATION.md` - API仕様
- `06_CODE_EXAMPLES.md` - コード例

### 4. Phase 2から開始（ASTノード拡張は既に開始済み）
- `src/parser/ast.hpp` にノード追加
- `src/parser/simple_parser.cpp` にパース処理追加

### 5. 進捗記録
`07_PROGRESS.md` を作成して進捗を記録

## 重要な注意事項

### 破壊的変更
- 既存の組み込みルール使用者は影響を受ける
- メジャーバージョンアップ（v2.0）として扱う
- マイグレーションガイドを充実させる

### 開発方針
- 素早く実装、素早くテスト
- 完璧を目指さず動くものを優先
- 段階的にコミット
- ドキュメントも同時に更新

### コミットメッセージ例
```
feat(lua-api): Add get_files() API

- Implement lua_get_files in lua_bridge.cpp
- Return array of file paths to Lua
- Support filtering by pattern

Related: Lua Migration Phase 3

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

## 次のステップ

1. ✅ 計画ドキュメント作成（完了）
2. [ ] `04_IMPLEMENTATION_GUIDE.md` を読む
3. [ ] Phase 2の実装開始（ASTノード追加）
4. [ ] Phase 3の実装（Lua API追加）
5. [ ] 動作確認とテスト

---

**頑張りましょう！このプロジェクトでcclintが真のLua-first linterになります。**
