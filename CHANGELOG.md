# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Lua汎用ノードアクセスAPI（get_node_type, get_node_name, get_node_location, get_children, get_parent, get_source_range）
- 自動修正機能（--fix, --fix-preview）
- プロファイリングモード（--profile）
- 依存関係追跡（DependencyTracker）
- データフロー解析（DataFlowAnalyzer）
- プラグインシステム（PluginLoader）
- LSPサーバー（cclint-lsp）
- Windows対応（ビルドガイド、MSVC サポート）
- 102個の標準Luaルールスクリプト
- 独自C++パーサー（SimpleParser）
- キャッシュシステム（FileCache）
- 並列処理（ThreadPool）
- インクリメンタル解析

### Changed
- main.cpp統合完了（全モジュール統合）

### Fixed
- (バグ修正は今後追加)

## [0.1.0-alpha] - 2026-01-24

### Added
- 初期実装
- CLI Module (ArgumentParser, HelpFormatter)
- Config Module (ConfigLoader, YamlConfig)
- Compiler Module (CompilerWrapper, CompilerDetector)
- Parser Module (AST, Lexer, SimpleParser)
- Diagnostic Module (Diagnostic, DiagnosticEngine)
- Output Module (TextFormatter, JsonFormatter, XmlFormatter)
- Utils Module (FileUtils, StringUtils, Logger)
- Rules Module (RuleBase, RuleRegistry, RuleExecutor)
- Engine Module (AnalysisEngine)
- Lua Module (LuaEngine, LuaBridge, LuaRule)
- 組み込みルール（NamingConvention, HeaderGuard, MaxLineLength, FunctionComplexity）

### Documentation
- README.md
- docs/requirements.md
- docs/design.md
- docs/detailed_design.md
- docs/milestones.md
- docs/TODO.md
- docs/build.md
- docs/usage.md
- docs/lua_api.md
- docs/troubleshooting.md
- docs/FAQ.md
- docs/plugin_development.md
- docs/windows_build.md
- CONTRIBUTING.md
- CLAUDE.md

[Unreleased]: https://github.com/KikuchiTomo/cclint/compare/v0.1.0-alpha...HEAD
[0.1.0-alpha]: https://github.com/KikuchiTomo/cclint/releases/tag/v0.1.0-alpha
