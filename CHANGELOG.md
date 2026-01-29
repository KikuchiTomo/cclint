# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive AST with 18 new node types (Constructor, Destructor, Lambda, Friend, Operator, Template, etc.)
- 18 new Lua APIs for complete C++ information access
- Builtin parser (renamed from SimpleParser)
- Call graph analysis for function call tracking
- Inheritance tree API
- Cache system (FileCache)
- Thread pool for parallel processing
- Incremental analysis support
- LSP server (cclint-lsp)
- Auto-fix functionality (--fix, --fix-preview)
- Profiling mode (--profile)
- Dependency tracking (DependencyTracker)
- Dataflow analysis (DataFlowAnalyzer)
- Plugin system (PluginLoader)

### Changed
- Renamed SimpleParser to BuiltinParser for clarity
- README simplified to essential information only
- All documentation now in English

### Removed
- Verbose design documents (10+ files, ~10,000 lines)
- Generic documentation (FAQ, troubleshooting, CI integration guides)
- Redundant information from README

### Fixed
- Multiple Lua rules interference (g_bridge pointer issue)
- Duplicate stub implementations in lua_bridge.cpp
- Constructor/Destructor detection in builtin parser
- Friend node field naming
- Operator detection (concatenate "operator" + symbol)

## [0.1.0-alpha] - 2026-01-24

### Added
- Initial implementation
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
- Builtin rules (NamingConvention, HeaderGuard, MaxLineLength, FunctionComplexity)

[Unreleased]: https://github.com/KikuchiTomo/cclint/compare/v0.1.0-alpha...HEAD
[0.1.0-alpha]: https://github.com/KikuchiTomo/cclint/releases/tag/v0.1.0-alpha
