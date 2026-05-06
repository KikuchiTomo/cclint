//! libclang の上に薄いラッパを敷き、Lua から扱いやすい AST ノード抽象を
//! 提供する。`clang::Entity` をそのまま見せると寿命が複雑なので、
//! libclang から取れる情報を可能な限り `OwnedNode` に詰めておく。

use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::rc::Rc;

use anyhow::{Context, Result};
use cclint_diagnostic::{Diagnostic, Severity, Span};
use serde::{Deserialize, Serialize};

/// libclang の Clang/Index/TranslationUnit を保持する解析セッション。
pub struct Session {
    _clang: Rc<clang::Clang>,
}

/// compile_commands.json から各ファイルのコンパイラ引数を引くためのラッパ。
/// libclang の `CompilationDatabase` を使わず serde_json で自前パースする。
pub struct CompilationDatabase {
    by_file: std::collections::HashMap<PathBuf, Vec<String>>,
}

#[derive(serde::Deserialize)]
struct CCEntry {
    directory: String,
    file: String,
    #[serde(default)]
    arguments: Option<Vec<String>>,
    #[serde(default)]
    command: Option<String>,
}

impl CompilationDatabase {
    /// 単一の compile_commands.json ファイルを読み込む。
    pub fn from_file(json_path: &Path) -> Result<Self> {
        let mut db = Self {
            by_file: std::collections::HashMap::new(),
        };
        db.add_file(json_path)?;
        Ok(db)
    }

    /// `<dir>/compile_commands.json` を読み込む。
    pub fn open(dir: &Path) -> Result<Self> {
        Self::from_file(&dir.join("compile_commands.json"))
    }

    /// 複数の compile_commands.json をマージして読み込む．後勝ち．
    pub fn from_files(json_paths: &[PathBuf]) -> Result<Self> {
        let mut db = Self {
            by_file: std::collections::HashMap::new(),
        };
        for p in json_paths {
            if let Err(e) = db.add_file(p) {
                eprintln!("warning: compile_commands {} 読込失敗: {e}", p.display());
            }
        }
        Ok(db)
    }

    fn add_file(&mut self, p: &Path) -> Result<()> {
        let text =
            std::fs::read_to_string(p).with_context(|| format!("読込失敗: {}", p.display()))?;
        let entries: Vec<CCEntry> = serde_json::from_str(&text)
            .with_context(|| format!("JSON parse 失敗: {}", p.display()))?;
        for e in entries {
            let dir = PathBuf::from(&e.directory);
            let file_path = if Path::new(&e.file).is_absolute() {
                PathBuf::from(&e.file)
            } else {
                dir.join(&e.file)
            };
            let raw_args = if let Some(args) = e.arguments {
                args
            } else if let Some(cmd) = e.command {
                shell_split(&cmd)
            } else {
                continue;
            };
            self.by_file.insert(
                file_path.canonicalize().unwrap_or(file_path),
                filter_args(&raw_args, &PathBuf::from(&e.file)),
            );
        }
        Ok(())
    }

    /// 与えられたソースファイルに対するコンパイラ引数を取り出す。
    /// 直接の登録が無ければ最近傍のソースファイルの引数で代用する
    /// (ヘッダは compile_commands.json に通常含まれないため)。
    pub fn arguments_for(&self, file: &Path) -> Option<Vec<String>> {
        let canon = file.canonicalize().unwrap_or(file.to_path_buf());
        if let Some(args) = self.by_file.get(&canon) {
            return Some(args.clone());
        }
        self.nearest_args(&canon)
    }

    /// ヘッダ用フォールバック: 同じディレクトリ → 兄弟 (include↔src) →
    /// 親ディレクトリの順で最も近い登録済みソースを探す．clangd の挙動を模倣．
    fn nearest_args(&self, file: &Path) -> Option<Vec<String>> {
        // 1. 同一ディレクトリ
        if let Some(parent) = file.parent() {
            for (k, v) in &self.by_file {
                if k.parent() == Some(parent) {
                    return Some(v.clone());
                }
            }
        }
        // 2. 兄弟: include/<X> ⇄ src/<X> 等の典型パスを試す
        for (k, v) in &self.by_file {
            if same_module(file, k) {
                return Some(v.clone());
            }
        }
        // 3. 親方向: 上位ディレクトリで一番深く一致するエントリ
        let mut cur = file.parent();
        let mut best: Option<(usize, Vec<String>)> = None;
        while let Some(d) = cur {
            for (k, v) in &self.by_file {
                if k.starts_with(d) {
                    let depth = d.components().count();
                    if best.as_ref().is_none_or(|(b, _)| depth > *b) {
                        best = Some((depth, v.clone()));
                    }
                }
            }
            if best.is_some() {
                return best.map(|(_, v)| v);
            }
            cur = d.parent();
        }
        // 4. 何でもいいので 1 個あれば返す (最後の砦)
        self.by_file.values().next().cloned()
    }

    pub fn entry_count(&self) -> usize {
        self.by_file.len()
    }
}

/// ヘッダ A とソース B が同一モジュールに属するかの粗い判定．
/// 例: foo/include/m/x.hpp と foo/src/m/x.cpp は true．
fn same_module(header: &Path, source: &Path) -> bool {
    let h_stem = match header.file_stem() {
        Some(s) => s.to_string_lossy().to_string(),
        None => return false,
    };
    let s_stem = match source.file_stem() {
        Some(s) => s.to_string_lossy().to_string(),
        None => return false,
    };
    if h_stem != s_stem {
        return false;
    }
    // 共通の親ディレクトリを持つか
    let h_parent = header.parent();
    let s_parent = source.parent();
    if h_parent == s_parent {
        return true;
    }
    // include/ と src/ を相互置換して一致するか
    if let (Some(hp), Some(sp)) = (h_parent, s_parent) {
        let hp_str = hp.to_string_lossy().to_string();
        let sp_str = sp.to_string_lossy().to_string();
        if hp_str.replace("/include/", "/src/") == sp_str
            || sp_str.replace("/src/", "/include/") == hp_str
        {
            return true;
        }
    }
    false
}

/// 簡易 shell split (空白区切り + ダブルクォート対応)
fn shell_split(s: &str) -> Vec<String> {
    let mut out = Vec::new();
    let mut cur = String::new();
    let mut in_quote = false;
    let mut chars = s.chars().peekable();
    while let Some(c) = chars.next() {
        match c {
            '"' => in_quote = !in_quote,
            '\\' if in_quote => {
                if let Some(next) = chars.next() {
                    cur.push(next);
                }
            }
            ' ' | '\t' if !in_quote => {
                if !cur.is_empty() {
                    out.push(std::mem::take(&mut cur));
                }
            }
            _ => cur.push(c),
        }
    }
    if !cur.is_empty() {
        out.push(cur);
    }
    out
}

/// compile_commands.json のコマンドから linter に有用な引数だけ残す。
fn filter_args(raw: &[String], target: &Path) -> Vec<String> {
    let mut out = Vec::new();
    let mut iter = raw.iter().peekable();
    // 最初の要素 (compiler 実行ファイル名) はスキップ
    iter.next();
    while let Some(a) = iter.next() {
        match a.as_str() {
            "-c" | "-o" => {
                // -c file / -o foo.o の引数も飛ばす
                iter.next();
            }
            // 出力依存の引数も除去
            "-MD" | "-MMD" | "-MP" => {}
            "-MF" | "-MT" | "-MQ" => {
                iter.next();
            }
            s if s.ends_with(".cpp")
                || s.ends_with(".cc")
                || s.ends_with(".cxx")
                || s.ends_with(".c")
                || s.ends_with(".m")
                || s.ends_with(".mm") =>
            {
                // 入力ソースファイル名は除く
                if Path::new(s) == target
                    || std::path::PathBuf::from(s).file_name() == target.file_name()
                {
                    continue;
                }
                out.push(a.clone());
            }
            _ => out.push(a.clone()),
        }
    }
    out
}

impl Session {
    pub fn new() -> Result<Self> {
        let c = clang::Clang::new().map_err(|e| anyhow::anyhow!("libclang 初期化失敗: {e}"))?;
        Ok(Self { _clang: Rc::new(c) })
    }

    /// 単一ファイルを解析し、AST ルートと診断を返す。
    pub fn parse_file(
        &self,
        path: &Path,
        cpp_standard: &str,
        extra_args: &[String],
    ) -> Result<(OwnedNode, Vec<Diagnostic>)> {
        let index = clang::Index::new(&self._clang, false, false);
        let std_arg = format!("-std={}", cpp_standard);
        // extra_args が compile_commands.json から来てる場合は -std や -x が
        // 既に含まれている．重複指定すると libclang が SEGV することがあるので
        // extra にあるなら自分では追加しない．
        let extra_has_std = extra_args.iter().any(|a| a.starts_with("-std="));
        let extra_has_lang = extra_args.iter().any(|a| a == "-x");
        let mut args: Vec<&str> = Vec::new();
        if !extra_has_std {
            args.push(&std_arg);
        }
        if !extra_has_lang {
            args.push("-x");
            args.push("c++");
        }
        args.push("-Wno-pragma-once-outside-header");
        for a in extra_args {
            args.push(a);
        }
        let tu = index
            .parser(path)
            .arguments(&args)
            .detailed_preprocessing_record(true)
            .skip_function_bodies(false)
            .parse()
            .with_context(|| format!("解析失敗: {}", path.display()))?;

        let mut diags = Vec::new();
        let mut has_fatal = false;
        for d in tu.get_diagnostics() {
            let sev = match d.get_severity() {
                clang::diagnostic::Severity::Fatal => {
                    has_fatal = true;
                    Severity::Error
                }
                clang::diagnostic::Severity::Error => Severity::Error,
                clang::diagnostic::Severity::Warning => Severity::Warning,
                clang::diagnostic::Severity::Note => Severity::Info,
                clang::diagnostic::Severity::Ignored => continue,
            };
            let loc = d.get_location().get_file_location();
            let span = loc.file.map(|f| Span {
                file: f.get_path(),
                byte_start: loc.offset as usize,
                byte_end: loc.offset as usize,
                line: loc.line,
                column: loc.column,
            });
            let mut diag = Diagnostic::new("clang", sev, d.get_text());
            if let Some(s) = span {
                diag = diag.with_span(s);
            }
            diags.push(diag);
        }

        // fatal error 出てる時点で AST が壊れている．古い libclang で
        // from_entity が SEGV することがあるため，AST 走査をスキップして
        // ダミーの空 TU ノードを返す．
        if has_fatal {
            let root = OwnedNode {
                kind: "TranslationUnit".to_string(),
                name: path.display().to_string(),
                span: Some(Span {
                    file: path.to_path_buf(),
                    byte_start: 0,
                    byte_end: 0,
                    line: 1,
                    column: 1,
                }),
                ..Default::default()
            };
            return Ok((root, diags));
        }

        let mut root = OwnedNode::from_entity(&tu.get_entity(), path);
        root.kind = "TranslationUnit".to_string();
        if root.span.is_none() {
            root.span = Some(Span {
                file: path.to_path_buf(),
                byte_start: 0,
                byte_end: 0,
                line: 1,
                column: 1,
            });
        }
        Ok((root, diags))
    }
}

/// libclang から取れる情報を可能な限り保持した値型ノード。
/// 別エンティティへの参照は USR 文字列で持つ (Lua から project index 経由で
/// 引ける形にする)。
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct OwnedNode {
    // ---- 基本 ----
    pub kind: String,
    pub name: String,
    pub display_name: String,
    pub spelling: String,
    pub usr: String,
    pub mangled_name: Option<String>,

    // ---- 型情報 ----
    pub type_name: Option<String>,
    pub canonical_type: Option<String>,
    pub typedef_underlying_type: Option<String>,
    pub result_type: Option<String>,
    pub enum_underlying_type: Option<String>,
    pub enum_constant_value: Option<i64>,
    pub bit_field_width: Option<usize>,
    pub offset_of_field: Option<usize>,

    // ---- linkage / 可視性 / ストレージ ----
    pub access: Option<String>,
    pub linkage: Option<String>,
    pub visibility: Option<String>,
    pub language: Option<String>,
    pub storage_class: Option<String>,
    pub tls_kind: Option<String>,
    pub availability: Option<String>,

    // ---- predicates ----
    pub is_definition: bool,
    pub is_declaration: bool,
    pub is_const: bool,  // const method
    pub is_static: bool, // static method
    pub is_virtual: bool,
    pub is_pure_virtual: bool,
    pub is_default_constructor: bool,
    pub is_copy_constructor: bool,
    pub is_move_constructor: bool,
    pub is_converting_constructor: bool,
    pub is_defaulted: bool,
    pub is_function_inlined: bool,
    pub is_variadic: bool,
    pub is_anonymous: bool,
    pub is_anonymous_record_decl: bool,
    pub is_abstract_record: bool,
    pub is_bit_field: bool,
    pub is_mutable: bool,
    pub is_scoped: bool, // enum class
    pub is_in_main_file: bool,
    pub is_in_system_header: bool,
    pub is_invalid_declaration: bool,
    pub is_attribute: bool,
    pub is_expression: bool,
    pub is_preprocessing: bool,
    pub is_reference: bool,
    pub is_statement: bool,
    pub is_unexposed: bool,
    pub is_function_like_macro: bool,
    pub is_builtin_macro: bool,
    pub is_inline_namespace: bool,
    pub is_dynamic_call: bool,
    pub is_macro_expansion: bool,

    // ---- 他エンティティへの参照 (USR) ----
    pub referenced_name: Option<String>,
    pub referenced_usr: Option<String>,
    pub semantic_parent_usr: Option<String>,
    pub lexical_parent_usr: Option<String>,
    pub definition_usr: Option<String>,
    pub canonical_usr: Option<String>,
    pub overridden_method_usrs: Vec<String>,

    // ---- include 情報 ----
    pub included_file: Option<PathBuf>,

    // ---- コメント ----
    pub comment: Option<String>,
    pub comment_brief: Option<String>,

    // ---- 位置 ----
    pub span: Option<Span>,
    pub spelling_span: Option<Span>,

    // ---- 子ノード ----
    pub children: Vec<OwnedNode>,
}

fn entity_usr(e: &clang::Entity) -> Option<String> {
    e.get_usr().map(|u| u.0)
}

impl OwnedNode {
    fn from_entity(e: &clang::Entity, primary_file: &Path) -> Self {
        let kind_str = format!("{:?}", e.get_kind());
        // span = file_location 系 (ユーザの編集箇所)
        let span = e.get_range().map(|r| {
            let s = r.get_start().get_file_location();
            let end = r.get_end().get_file_location();
            Span {
                file: s
                    .file
                    .map(|f| f.get_path())
                    .unwrap_or_else(|| primary_file.to_path_buf()),
                byte_start: s.offset as usize,
                byte_end: end.offset as usize,
                line: s.line,
                column: s.column,
            }
        });
        // spelling_span = マクロ展開元 (マクロ定義側)
        let spelling_span = e.get_range().map(|r| {
            let s = r.get_start().get_spelling_location();
            let end = r.get_end().get_spelling_location();
            Span {
                file: s
                    .file
                    .map(|f| f.get_path())
                    .unwrap_or_else(|| primary_file.to_path_buf()),
                byte_start: s.offset as usize,
                byte_end: end.offset as usize,
                line: s.line,
                column: s.column,
            }
        });
        let is_macro_expansion = match (&span, &spelling_span) {
            (Some(a), Some(b)) => a.file != b.file || a.byte_start != b.byte_start,
            _ => false,
        };

        // 以下は宣言のみで安全に呼べる．Decl 系でないと SEGV するものがある．
        let is_decl = e.is_declaration();
        let access = e
            .get_accessibility()
            .map(|a| format!("{a:?}").to_lowercase());
        let linkage = if is_decl {
            e.get_linkage().map(|x| format!("{x:?}").to_lowercase())
        } else {
            None
        };
        let visibility = if is_decl {
            e.get_visibility().map(|x| format!("{x:?}").to_lowercase())
        } else {
            None
        };
        let language = if is_decl {
            e.get_language().map(|x| format!("{x:?}").to_lowercase())
        } else {
            None
        };
        let storage_class = if is_decl {
            e.get_storage_class()
                .map(|x| format!("{x:?}").to_lowercase())
        } else {
            None
        };
        let tls_kind = if is_decl {
            e.get_tls_kind().map(|x| format!("{x:?}").to_lowercase())
        } else {
            None
        };
        let availability = if is_decl {
            Some(format!("{:?}", e.get_availability()).to_lowercase())
        } else {
            None
        };

        let ty = e.get_type();
        let type_name = ty.as_ref().map(|t| t.get_display_name());
        // get_canonical_type / typedef_underlying_type / result_type /
        // enum_underlying_type は kind が合わない cursor で呼ぶと SEGV する
        // ことがあるため kind ガードする．
        let canonical_type = if e.is_declaration() || e.is_expression() {
            ty.as_ref()
                .map(|t| t.get_canonical_type().get_display_name())
        } else {
            None
        };
        let typedef_underlying_type =
            if matches!(kind_str.as_str(), "TypedefDecl" | "TypeAliasDecl") {
                e.get_typedef_underlying_type()
                    .map(|t| t.get_display_name())
            } else {
                None
            };
        let result_type = if is_function_kind(&kind_str) {
            e.get_result_type().map(|t| t.get_display_name())
        } else {
            None
        };
        let enum_underlying_type = if kind_str == "EnumDecl" {
            e.get_enum_underlying_type().map(|t| t.get_display_name())
        } else {
            None
        };
        let enum_constant_value = if kind_str == "EnumConstantDecl" {
            e.get_enum_constant_value().map(|(v, _u)| v)
        } else {
            None
        };
        // get_bit_field_width / get_offset_of_field は FieldDecl 以外で呼ぶと
        // libclang が SEGV することがあるため kind ガードする．
        let (bit_field_width, offset_of_field) = if kind_str == "FieldDecl" {
            (e.get_bit_field_width(), e.get_offset_of_field().ok())
        } else {
            (None, None)
        };

        // get_reference は Expr/Reference 系で安全．Decl/Stmt 等で呼ぶと SEGV する．
        let referenced = if e.is_expression()
            || e.is_reference()
            || matches!(kind_str.as_str(), "InclusionDirective" | "MacroExpansion")
        {
            e.get_reference()
        } else {
            None
        };
        let referenced_name = referenced.as_ref().and_then(|r| r.get_name());
        let referenced_usr = referenced.as_ref().and_then(|r| entity_usr(r));

        // Decl 系のみで安全
        let (semantic_parent_usr, lexical_parent_usr, definition_usr, canonical_usr) = if is_decl {
            (
                e.get_semantic_parent().and_then(|p| entity_usr(&p)),
                e.get_lexical_parent().and_then(|p| entity_usr(&p)),
                e.get_definition().and_then(|d| entity_usr(&d)),
                entity_usr(&e.get_canonical_entity()),
            )
        } else {
            (None, None, None, None)
        };
        // get_overridden_methods は Method 以外で呼ぶと SEGV することがある
        let overridden_method_usrs: Vec<String> =
            if matches!(kind_str.as_str(), "Method" | "CXXMethodDecl") {
                e.get_overridden_methods()
                    .map(|v| v.into_iter().filter_map(|x| entity_usr(&x)).collect())
                    .unwrap_or_default()
            } else {
                Vec::new()
            };

        let included_file = e.get_file().map(|f| f.get_path());

        let (comment, comment_brief) = if is_decl {
            (e.get_comment(), e.get_comment_brief())
        } else {
            (None, None)
        };

        OwnedNode {
            kind: format!("{:?}", e.get_kind()),
            name: e.get_name().unwrap_or_default(),
            display_name: e.get_display_name().unwrap_or_default(),
            spelling: e.get_name().unwrap_or_default(),
            usr: entity_usr(e).unwrap_or_default(),
            mangled_name: if is_decl { e.get_mangled_name() } else { None },

            type_name,
            canonical_type,
            typedef_underlying_type,
            result_type,
            enum_underlying_type,
            enum_constant_value,
            bit_field_width,
            offset_of_field,

            access,
            linkage,
            visibility,
            language,
            storage_class,
            tls_kind,
            availability,

            is_definition: e.is_definition(),
            is_declaration: e.is_declaration(),
            is_const: e.is_const_method(),
            is_static: e.is_static_method(),
            is_virtual: e.is_virtual_method(),
            is_pure_virtual: e.is_pure_virtual_method(),
            is_default_constructor: e.is_default_constructor(),
            is_copy_constructor: e.is_copy_constructor(),
            is_move_constructor: e.is_move_constructor(),
            is_converting_constructor: e.is_converting_constructor(),
            is_defaulted: e.is_defaulted(),
            is_function_inlined: e.is_inline_function(),
            is_variadic: e.is_variadic(),
            is_anonymous: e.is_anonymous(),
            is_anonymous_record_decl: e.is_anonymous_record_decl(),
            is_abstract_record: e.is_abstract_record(),
            is_bit_field: e.is_bit_field(),
            is_mutable: e.is_mutable(),
            is_scoped: e.is_scoped(),
            is_in_main_file: e.is_in_main_file(),
            is_in_system_header: e.is_in_system_header(),
            is_invalid_declaration: e.is_invalid_declaration(),
            is_attribute: e.is_attribute(),
            is_expression: e.is_expression(),
            is_preprocessing: e.is_preprocessing(),
            is_reference: e.is_reference(),
            is_statement: e.is_statement(),
            is_unexposed: e.is_unexposed(),
            is_function_like_macro: e.is_function_like_macro(),
            is_builtin_macro: e.is_builtin_macro(),
            is_inline_namespace: e.is_inline_namespace(),
            is_dynamic_call: e.is_dynamic_call(),
            is_macro_expansion,

            referenced_name,
            referenced_usr,
            semantic_parent_usr,
            lexical_parent_usr,
            definition_usr,
            canonical_usr,
            overridden_method_usrs,

            included_file,
            comment,
            comment_brief,

            span,
            spelling_span,

            children: e
                .get_children()
                .iter()
                .map(|c| OwnedNode::from_entity(c, primary_file))
                .collect(),
        }
    }

    /// 深さ優先で全ノードを訪問する。
    pub fn walk<F: FnMut(&OwnedNode)>(&self, f: &mut F) {
        f(self);
        for c in &self.children {
            c.walk(f);
        }
    }
}

/// プロジェクト全体の AST を集約した USR ベースのインデックス。
/// 「特定の関数を呼んでいるのは誰か」「ある宣言の定義はどこか」を
/// 複数 TU を跨いで引けるようにする。
#[derive(Debug, Clone, Default, Serialize)]
pub struct ProjectIndex {
    /// USR -> 定義ノードの参照群 (前方宣言など複数の場合あり)
    pub definitions: HashMap<String, Vec<NodeRef>>,
    /// USR -> その USR を参照している箇所 (CallExpr, DeclRefExpr, BaseSpecifier, etc.)
    pub references: HashMap<String, Vec<NodeRef>>,
    /// 解析した全ファイル
    pub files: Vec<PathBuf>,
}

#[derive(Debug, Clone, Serialize)]
pub struct NodeRef {
    pub kind: String,
    pub name: String,
    pub usr: String,
    pub file: PathBuf,
    pub line: u32,
    pub column: u32,
    pub byte_start: usize,
    pub byte_end: usize,
    /// この参照を囲っている関数 (FunctionDecl/Method/Constructor/Destructor) の情報。
    /// caller として使う。
    pub enclosing_function_name: Option<String>,
    pub enclosing_function_usr: Option<String>,
}

fn is_function_kind(kind: &str) -> bool {
    matches!(
        kind,
        "FunctionDecl"
            | "Method"
            | "CXXMethodDecl"
            | "Constructor"
            | "Destructor"
            | "FunctionTemplate"
            | "ConversionFunction"
    )
}

impl ProjectIndex {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_root(&mut self, file: &Path, root: &OwnedNode) {
        self.files.push(file.to_path_buf());
        walk_index(root, None, None, self);
    }
}

fn walk_index(
    node: &OwnedNode,
    enclosing_usr: Option<&str>,
    enclosing_name: Option<&str>,
    idx: &mut ProjectIndex,
) {
    let (cur_enclosing_usr, cur_enclosing_name): (Option<String>, Option<String>) =
        if is_function_kind(&node.kind) && !node.usr.is_empty() {
            (Some(node.usr.clone()), Some(node.name.clone()))
        } else {
            (
                enclosing_usr.map(|s| s.to_string()),
                enclosing_name.map(|s| s.to_string()),
            )
        };

    if let Some(span) = &node.span {
        let nref = NodeRef {
            kind: node.kind.clone(),
            name: node.name.clone(),
            usr: node.usr.clone(),
            file: span.file.clone(),
            line: span.line,
            column: span.column,
            byte_start: span.byte_start,
            byte_end: span.byte_end,
            enclosing_function_name: cur_enclosing_name.clone(),
            enclosing_function_usr: cur_enclosing_usr.clone(),
        };
        if node.is_definition && !node.usr.is_empty() {
            idx.definitions
                .entry(node.usr.clone())
                .or_default()
                .push(nref.clone());
        }
        if let Some(ref_usr) = &node.referenced_usr {
            if !ref_usr.is_empty() {
                idx.references
                    .entry(ref_usr.clone())
                    .or_default()
                    .push(nref);
            }
        }
    }

    for c in &node.children {
        walk_index(
            c,
            cur_enclosing_usr.as_deref(),
            cur_enclosing_name.as_deref(),
            idx,
        );
    }
}
