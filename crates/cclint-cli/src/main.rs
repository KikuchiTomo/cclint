use std::path::PathBuf;
use std::process::ExitCode;

use anyhow::{Context, Result};
use clap::Parser;

use cclint_ast::Session;
use cclint_config::Config;
use cclint_diagnostic::{Diagnostic, Severity};
use cclint_script::Engine;

#[derive(Parser, Debug)]
#[command(
    name = "cclint",
    version,
    about = "Customizable C++ Linter (libclang + Lua)"
)]
struct Args {
    /// 解析対象のルートディレクトリ
    #[arg(default_value = ".")]
    root: PathBuf,

    /// 設定ファイル (.cclint.toml)
    #[arg(short = 'c', long, default_value = ".cclint.toml")]
    config: PathBuf,

    /// 追加で読み込む Lua ルール (繰り返し可)
    #[arg(short = 'r', long = "rule")]
    extra_rules: Vec<PathBuf>,

    /// 出力形式
    #[arg(long, value_enum, default_value_t = Format::Text)]
    format: Format,

    /// 警告も終了コード非ゼロとして扱う
    #[arg(long)]
    werror: bool,
}

#[derive(clap::ValueEnum, Clone, Debug)]
enum Format {
    Text,
    Json,
}

fn main() -> ExitCode {
    tracing_subscriber::fmt()
        .with_env_filter(tracing_subscriber::EnvFilter::from_default_env())
        .with_target(false)
        .init();

    setup_bundled_libclang();

    match run() {
        Ok(code) => code,
        Err(e) => {
            eprintln!("cclint: エラー: {e:#}");
            ExitCode::from(2)
        }
    }
}

/// バイナリの隣 (`<bin>/../lib/libclang.*`) に同梱された libclang があれば
/// LIBCLANG_PATH を自動設定する．ユーザが明示的に LIBCLANG_PATH を指定して
/// いれば尊重する．
fn setup_bundled_libclang() {
    if std::env::var_os("LIBCLANG_PATH").is_some() {
        return;
    }
    let exe = match std::env::current_exe() {
        Ok(p) => p,
        Err(_) => return,
    };
    let lib_dir = match exe.parent().and_then(|p| p.parent()) {
        Some(parent) => parent.join("lib"),
        None => return,
    };
    if !lib_dir.is_dir() {
        return;
    }
    let candidates = [
        "libclang.dylib",
        "libclang.so",
        "libclang.so.1",
        "libclang-18.so.18.1",
        "libclang.so.18",
        "libclang.so.17",
        "libclang.so.14",
    ];
    for name in candidates {
        if lib_dir.join(name).exists() {
            std::env::set_var("LIBCLANG_PATH", &lib_dir);
            return;
        }
    }
    // glob: libclang.so.*
    if let Ok(rd) = std::fs::read_dir(&lib_dir) {
        for entry in rd.flatten() {
            let n = entry.file_name();
            let s = n.to_string_lossy();
            if s.starts_with("libclang") && (s.contains(".so") || s.ends_with(".dylib")) {
                std::env::set_var("LIBCLANG_PATH", &lib_dir);
                return;
            }
        }
    }
}

fn run() -> Result<ExitCode> {
    let args = Args::parse();
    let config_path = args.config.clone();
    let cfg = Config::load(&config_path).with_context(|| "設定読込失敗")?;

    let root = args.root.canonicalize().unwrap_or(args.root.clone());
    let files = cfg.resolve_files(&root)?;

    let engine = Engine::new()?;
    for r in &cfg.rules {
        if !r.enabled {
            continue;
        }
        let p = if r.path.is_absolute() {
            r.path.clone()
        } else {
            root.join(&r.path)
        };
        engine.load_script(&p)?;
    }
    for p in &args.extra_rules {
        engine.load_script(p)?;
    }

    if engine.rule_count() == 0 {
        eprintln!(
            "cclint: 注意: 有効なルールが 0 個です。`-r path/to/rule.lua` か .cclint.toml の rules を設定してください。"
        );
    }

    let session = Session::new()?;
    let mut all: Vec<Diagnostic> = Vec::new();
    for f in &files {
        match session.parse_file(f, &cfg.cpp_standard, &[]) {
            Ok((ast, mut diags)) => {
                all.append(&mut diags);
                let mut rule_diags = engine.run(&ast)?;
                all.append(&mut rule_diags);
            }
            Err(e) => {
                all.push(Diagnostic::new(
                    "cclint",
                    Severity::Error,
                    format!("解析失敗: {}: {e}", f.display()),
                ));
            }
        }
    }

    // (rule, file, line, column, message) で重複を削除
    let mut seen = std::collections::HashSet::new();
    all.retain(|d| {
        let key = (
            d.rule.clone(),
            d.span.as_ref().map(|s| s.file.clone()),
            d.span.as_ref().map(|s| s.line).unwrap_or(0),
            d.span.as_ref().map(|s| s.column).unwrap_or(0),
            d.message.clone(),
        );
        seen.insert(key)
    });

    match args.format {
        Format::Text => cclint_diagnostic::emit_text(&all)?,
        Format::Json => cclint_diagnostic::emit_json(&all)?,
    }

    let has_error = all.iter().any(|d| d.severity == Severity::Error);
    let has_warn = all.iter().any(|d| d.severity == Severity::Warning);
    let fail = has_error || (args.werror && has_warn);
    Ok(if fail { ExitCode::from(1) } else { ExitCode::SUCCESS })
}
