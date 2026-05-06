//! 診断 (diagnostic) のデータ構造と整形出力。日本語メッセージ対応。

use std::path::PathBuf;

use codespan_reporting::diagnostic::{Diagnostic as CDiag, Label};
use codespan_reporting::files::SimpleFiles;
use codespan_reporting::term;
use codespan_reporting::term::termcolor::{ColorChoice, StandardStream};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Severity {
    Error,
    Warning,
    Info,
    Hint,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Span {
    pub file: PathBuf,
    pub byte_start: usize,
    pub byte_end: usize,
    pub line: u32,
    pub column: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Diagnostic {
    pub rule: String,
    pub severity: Severity,
    pub message: String,
    pub span: Option<Span>,
    /// 補足情報 (任意, 日本語可)
    #[serde(default)]
    pub notes: Vec<String>,
}

impl Diagnostic {
    pub fn new(rule: impl Into<String>, severity: Severity, message: impl Into<String>) -> Self {
        Self {
            rule: rule.into(),
            severity,
            message: message.into(),
            span: None,
            notes: vec![],
        }
    }

    pub fn with_span(mut self, span: Span) -> Self {
        self.span = Some(span);
        self
    }

    pub fn with_note(mut self, note: impl Into<String>) -> Self {
        self.notes.push(note.into());
        self
    }
}

/// 端末への整形出力 (codespan-reporting 経由)
pub fn emit_text(diags: &[Diagnostic]) -> std::io::Result<()> {
    let writer = StandardStream::stderr(ColorChoice::Auto);
    let cfg = term::Config::default();
    let mut files = SimpleFiles::new();
    let mut file_ids = std::collections::HashMap::new();

    for d in diags {
        let cdiag = match &d.span {
            Some(span) => {
                let id = *file_ids.entry(span.file.clone()).or_insert_with(|| {
                    let src = std::fs::read_to_string(&span.file).unwrap_or_default();
                    files.add(span.file.display().to_string(), src)
                });
                let label =
                    Label::primary(id, span.byte_start..span.byte_end).with_message(&d.message);
                let mut cd = match d.severity {
                    Severity::Error => CDiag::error(),
                    Severity::Warning => CDiag::warning(),
                    Severity::Info => CDiag::note(),
                    Severity::Hint => CDiag::help(),
                };
                cd = cd
                    .with_message(format!("[{}] {}", d.rule, d.message))
                    .with_labels(vec![label]);
                if !d.notes.is_empty() {
                    cd = cd.with_notes(d.notes.clone());
                }
                cd
            }
            None => {
                let mut cd = match d.severity {
                    Severity::Error => CDiag::error(),
                    Severity::Warning => CDiag::warning(),
                    Severity::Info => CDiag::note(),
                    Severity::Hint => CDiag::help(),
                };
                cd = cd.with_message(format!("[{}] {}", d.rule, d.message));
                if !d.notes.is_empty() {
                    cd = cd.with_notes(d.notes.clone());
                }
                cd
            }
        };
        term::emit(&mut writer.lock(), &cfg, &files, &cdiag).ok();
    }
    Ok(())
}

pub fn emit_json(diags: &[Diagnostic]) -> std::io::Result<()> {
    let s = serde_json::to_string_pretty(diags).unwrap();
    println!("{s}");
    Ok(())
}
