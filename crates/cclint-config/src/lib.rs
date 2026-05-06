//! cclint configuration loader.
//!
//! Reads `.cclint.toml` and resolves include/exclude globs into a list of
//! C++ source files to lint.

use std::path::{Path, PathBuf};

use anyhow::{Context, Result};
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Config {
    #[serde(default = "default_cpp_standard")]
    pub cpp_standard: String,

    #[serde(default = "default_includes")]
    pub include_patterns: Vec<String>,

    #[serde(default)]
    pub exclude_patterns: Vec<String>,

    #[serde(default)]
    pub compile_commands: Option<PathBuf>,

    /// libclang に渡すコンパイラ引数 (compile_commands.json が無いときの代替)．
    /// 例: ["-I", "include", "-DDEBUG=1"]
    #[serde(default)]
    pub extra_args: Vec<String>,

    #[serde(default)]
    pub rules: Vec<RuleEntry>,

    #[serde(default)]
    pub suppressions: Vec<Suppression>,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Suppression {
    /// 対象ファイルの glob (相対 or 絶対) のリスト。空 = 全ファイル。
    #[serde(default)]
    pub files: Vec<String>,
    /// 対象ルール名のリスト。空 or "*" を含む = 全ルール。
    #[serde(default)]
    pub rules: Vec<String>,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct RuleEntry {
    pub path: PathBuf,
    #[serde(default = "default_true")]
    pub enabled: bool,
    #[serde(default)]
    pub severity: Option<String>,
    #[serde(default)]
    pub parameters: toml::Table,
}

fn default_true() -> bool {
    true
}
fn default_cpp_standard() -> String {
    "c++17".into()
}
fn default_includes() -> Vec<String> {
    vec![
        "**/*.cpp".into(),
        "**/*.cc".into(),
        "**/*.h".into(),
        "**/*.hpp".into(),
    ]
}

impl Config {
    /// Load `.cclint.toml` from `path`. If the file is absent, returns
    /// an empty default configuration (no rules, default include globs).
    pub fn load(path: &Path) -> Result<Self> {
        if !path.exists() {
            return Ok(Self::default());
        }
        let text = std::fs::read_to_string(path)
            .with_context(|| format!("読込失敗: {}", path.display()))?;
        let cfg: Self = toml::from_str(&text)
            .with_context(|| format!("TOML パース失敗: {}", path.display()))?;
        Ok(cfg)
    }

    pub fn resolve_files(&self, root: &Path) -> Result<Vec<PathBuf>> {
        let mut hits = Vec::new();
        let opts = glob::MatchOptions::default();
        for pat in &self.include_patterns {
            let full = root.join(pat).to_string_lossy().to_string();
            for entry in glob::glob_with(&full, opts)? {
                let p = entry?;
                if p.is_file() && !self.is_excluded(&p, root) {
                    hits.push(p);
                }
            }
        }
        hits.sort();
        hits.dedup();
        Ok(hits)
    }

    fn is_excluded(&self, p: &Path, root: &Path) -> bool {
        let rel = p.strip_prefix(root).unwrap_or(p);
        for pat in &self.exclude_patterns {
            if let Ok(g) = glob::Pattern::new(pat) {
                if g.matches_path(rel) || g.matches_path(p) {
                    return true;
                }
            }
        }
        false
    }
}

impl Default for Config {
    fn default() -> Self {
        Self {
            cpp_standard: default_cpp_standard(),
            include_patterns: default_includes(),
            exclude_patterns: vec![],
            compile_commands: None,
            extra_args: vec![],
            rules: vec![],
            suppressions: vec![],
        }
    }
}
