//! libclang の上に薄いラッパを敷き、Lua から扱いやすい AST ノード抽象を
//! 提供する。`clang::Entity` をそのまま見せると寿命が複雑なので、
//! ここでは `OwnedNode` というシリアライズ可能な値型に変換しておく。

use std::path::{Path, PathBuf};
use std::sync::Arc;

use anyhow::{Context, Result};
use cclint_diagnostic::{Diagnostic, Severity, Span};
use serde::Serialize;

/// libclang の Clang/Index/TranslationUnit を保持する解析セッション。
/// プロセス内で一つだけ作る (`clang::Clang::new()` は単一インスタンス)。
pub struct Session {
    _clang: Arc<clang::Clang>,
}

impl Session {
    pub fn new() -> Result<Self> {
        let c = clang::Clang::new()
            .map_err(|e| anyhow::anyhow!("libclang 初期化失敗: {e}"))?;
        Ok(Self { _clang: Arc::new(c) })
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
        // ヘッダを TU として parse すると "#pragma once in main file" 警告が出るので抑制。
        let mut args: Vec<&str> =
            vec![&std_arg, "-x", "c++", "-Wno-pragma-once-outside-header"];
        for a in extra_args {
            args.push(a);
        }
        let tu = index
            .parser(path)
            .arguments(&args)
            .detailed_preprocessing_record(true)
            .parse()
            .with_context(|| format!("解析失敗: {}", path.display()))?;

        let mut diags = Vec::new();
        for d in tu.get_diagnostics() {
            let sev = match d.get_severity() {
                clang::diagnostic::Severity::Fatal | clang::diagnostic::Severity::Error => {
                    Severity::Error
                }
                clang::diagnostic::Severity::Warning => Severity::Warning,
                clang::diagnostic::Severity::Note => Severity::Info,
                clang::diagnostic::Severity::Ignored => continue,
            };
            let loc = d.get_location().get_file_location();
            let span = loc.file.map(|f| Span {
                file: PathBuf::from(f.get_path()),
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

        let mut root = OwnedNode::from_entity(&tu.get_entity(), path);
        // libclang のバージョン差で TU の kind が NotImplemented になることがあるため固定。
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

/// libclang の Cursor/Entity から値型へ変換した AST ノード。
/// 必要十分な属性のみ保持する (Lua 側で過不足あれば追加する想定)。
#[derive(Debug, Clone, Serialize)]
pub struct OwnedNode {
    pub kind: String,
    pub name: String,
    pub display_name: String,
    pub spelling: String,
    pub usr: String,
    pub access: Option<String>,
    pub is_definition: bool,
    pub is_const: bool,
    pub is_static: bool,
    pub is_virtual: bool,
    pub is_pure_virtual: bool,
    pub type_name: Option<String>,
    /// このノードが参照する宣言の名前 (CallExpr → FunctionDecl 等)。
    pub referenced_name: Option<String>,
    /// 参照先の USR。同一の関数/宣言を一意に識別したいときに使う。
    pub referenced_usr: Option<String>,
    /// `#include` ディレクティブの場合、解決後のファイルパス。
    pub included_file: Option<PathBuf>,
    pub span: Option<Span>,
    pub children: Vec<OwnedNode>,
}

impl OwnedNode {
    fn from_entity(e: &clang::Entity, primary_file: &Path) -> Self {
        let kind = format!("{:?}", e.get_kind());
        let name = e.get_name().unwrap_or_default();
        let display_name = e.get_display_name().unwrap_or_default();
        let spelling = name.clone();
        let usr = e.get_usr().map(|u| u.0).unwrap_or_default();
        let access = e.get_accessibility().map(|a| format!("{a:?}").to_lowercase());
        let is_definition = e.is_definition();
        let is_const = e.is_const_method();
        let is_static = e.is_static_method();
        let is_virtual = e.is_virtual_method();
        let is_pure_virtual = e.is_pure_virtual_method();
        let type_name = e.get_type().map(|t| t.get_display_name());
        let referenced_name = e.get_reference().and_then(|r| r.get_name());
        let referenced_usr = e.get_reference().and_then(|r| r.get_usr().map(|u| u.0));
        let included_file = e.get_file().map(|f| PathBuf::from(f.get_path()));
        let span = e.get_range().map(|r| {
            let s = r.get_start().get_file_location();
            let end = r.get_end().get_file_location();
            Span {
                file: s.file.map(|f| PathBuf::from(f.get_path())).unwrap_or_else(|| primary_file.to_path_buf()),
                byte_start: s.offset as usize,
                byte_end: end.offset as usize,
                line: s.line,
                column: s.column,
            }
        });

        let children: Vec<OwnedNode> = e
            .get_children()
            .iter()
            .map(|c| OwnedNode::from_entity(c, primary_file))
            .collect();

        OwnedNode {
            kind,
            name,
            display_name,
            spelling,
            usr,
            access,
            is_definition,
            is_const,
            is_static,
            is_virtual,
            is_pure_virtual,
            type_name,
            referenced_name,
            referenced_usr,
            included_file,
            span,
            children,
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
