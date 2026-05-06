//! 診断のフィルタリング (suppression)。
//!
//! 1. ソース内コメント:
//!    `// cclint:disable=rule1,rule2`            … その行の指定ルール抑制
//!    `// cclint:disable`                        … その行の全ルール抑制
//!    `// cclint:disable-next-line=rule1`        … 次行の指定ルール抑制
//!    `// cclint:disable-next-line`              … 次行の全ルール抑制
//!    `// cclint:file-disable=rule1`             … ファイル全体で指定ルール抑制
//!    `// cclint:file-disable`                   … ファイル全体で全ルール抑制
//!
//! 2. 設定ファイル `.cclint.toml`:
//!    [[suppressions]]
//!    files = ["src/legacy/**"]
//!    rules = ["class-pascal-case", "function-snake-case"]
//!
//!    [[suppressions]]
//!    files = ["src/generated/**"]
//!    rules = ["*"]   # 全ルール

use std::collections::HashMap;
use std::path::{Path, PathBuf};

use cclint_config::Suppression;
use cclint_diagnostic::Diagnostic;

pub struct SuppressionFilter {
    sources: HashMap<PathBuf, Vec<String>>,
    config: Vec<(Vec<glob::Pattern>, Vec<String>)>,
}

impl SuppressionFilter {
    pub fn new(items: &[Suppression]) -> Self {
        let config = items
            .iter()
            .map(|s| {
                let globs: Vec<glob::Pattern> = s
                    .files
                    .iter()
                    .filter_map(|p| glob::Pattern::new(p).ok())
                    .collect();
                (globs, s.rules.clone())
            })
            .collect();
        Self {
            sources: HashMap::new(),
            config,
        }
    }

    pub fn is_suppressed(&mut self, diag: &Diagnostic) -> bool {
        let span = match &diag.span {
            Some(s) => s,
            None => return false,
        };

        // 設定ファイル経由の suppression
        for (globs, rules) in &self.config {
            let in_files = globs.is_empty()
                || globs
                    .iter()
                    .any(|g| g.matches_path(&span.file) || matches_relative(g, &span.file));
            let in_rules = rules.is_empty() || rules.iter().any(|r| r == "*" || r == &diag.rule);
            if in_files && in_rules {
                return true;
            }
        }

        // インラインコメント
        let lines = self.lines_of(&span.file);
        if lines.is_empty() {
            return false;
        }

        // file-disable: ファイル全体で抑制 (どこか 1 行にあれば有効)
        for line in lines.iter() {
            if let Some(rules) = parse_marker(line, "file-disable") {
                if rules_match(&rules, &diag.rule) {
                    return true;
                }
            }
        }

        let idx = (span.line as usize).saturating_sub(1);

        // 当該行の disable
        if idx < lines.len() {
            if let Some(rules) = parse_marker(&lines[idx], "disable") {
                if rules_match(&rules, &diag.rule) {
                    return true;
                }
            }
        }

        // 前の行の disable-next-line
        if idx > 0 {
            if let Some(rules) = parse_marker(&lines[idx - 1], "disable-next-line") {
                if rules_match(&rules, &diag.rule) {
                    return true;
                }
            }
        }

        false
    }

    fn lines_of(&mut self, p: &Path) -> &Vec<String> {
        self.sources.entry(p.to_path_buf()).or_insert_with(|| {
            std::fs::read_to_string(p)
                .map(|s| s.lines().map(|l| l.to_string()).collect())
                .unwrap_or_default()
        })
    }
}

fn matches_relative(g: &glob::Pattern, p: &Path) -> bool {
    if let Ok(cwd) = std::env::current_dir() {
        if let Ok(rel) = p.strip_prefix(&cwd) {
            return g.matches_path(rel);
        }
    }
    false
}

/// 行から `// cclint:<directive>` または `// cclint:<directive>=rule1,rule2`
/// を検出し，対象ルール名のリストを返す．None なら指示なし．
/// 空リストはワイルドカード (全ルール対象)．
fn parse_marker(line: &str, directive: &str) -> Option<Vec<String>> {
    // C-style 行コメントまたは /* ... */ 内に limited 対応
    let prefix = format!("cclint:{}", directive);
    let pos = line.find(&prefix)?;
    let rest = &line[pos + prefix.len()..];
    if let Some(eq) = rest.strip_prefix('=') {
        // ルール名はカンマ・空白区切り．次の空白か */ で終わる
        let end = eq
            .find(|c: char| c.is_whitespace() || c == '*')
            .unwrap_or(eq.len());
        let names: Vec<String> = eq[..end]
            .split(',')
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty())
            .collect();
        Some(names)
    } else {
        // 直後が空白か行末ならワイルドカード扱い (全ルール)
        let next = rest.chars().next();
        match next {
            None | Some(' ') | Some('\t') | Some('*') | Some('/') => Some(vec![]),
            _ => None,
        }
    }
}

fn rules_match(target_rules: &[String], rule: &str) -> bool {
    target_rules.is_empty() || target_rules.iter().any(|r| r == "*" || r == rule)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_disable_with_rules() {
        assert_eq!(
            parse_marker("    int x; // cclint:disable=rule-a,rule-b", "disable"),
            Some(vec!["rule-a".to_string(), "rule-b".to_string()])
        );
    }

    #[test]
    fn parse_disable_wildcard() {
        assert_eq!(parse_marker("// cclint:disable", "disable"), Some(vec![]));
    }

    #[test]
    fn parse_no_match() {
        assert_eq!(parse_marker("int x;", "disable"), None);
    }

    #[test]
    fn parse_disable_next_line() {
        assert_eq!(
            parse_marker("// cclint:disable-next-line=foo", "disable-next-line"),
            Some(vec!["foo".to_string()])
        );
    }
}
