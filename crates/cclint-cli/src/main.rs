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

    /// 進捗を表示する (どのファイルを parse 中か stderr に出す)
    #[arg(short = 'v', long)]
    verbose: bool,

    /// 内部用: 1 ファイルだけ parse し AST+診断を JSON で stdout に書き出す。
    /// 親プロセスが各ファイルをサブプロセスで parse することで，libclang が
    /// SEGV してもプロジェクト全体の lint が止まらないようにする．
    #[arg(long = "internal-parse", hide = true)]
    internal_parse: Option<PathBuf>,

    /// サブプロセス隔離を無効化 (デバッグ用)．通常 ON．
    #[arg(long, default_value_t = false)]
    no_subprocess: bool,
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

#[cfg(unix)]
fn signal_of(status: &std::process::ExitStatus) -> Option<i32> {
    use std::os::unix::process::ExitStatusExt;
    status.signal()
}
#[cfg(not(unix))]
fn signal_of(_: &std::process::ExitStatus) -> Option<i32> {
    None
}

/// 子プロセスとして cclint --internal-parse を起動して 1 ファイル parse させ，
/// JSON 結果を読む．SEGV 等で異常終了したら Err を返す．
fn parse_in_subprocess(
    exe: &std::path::Path,
    file: &std::path::Path,
    config: &std::path::Path,
) -> Result<(cclint_ast::OwnedNode, Vec<Diagnostic>)> {
    let output = std::process::Command::new(exe)
        .arg("--internal-parse")
        .arg(file)
        .arg("-c")
        .arg(config)
        .output()
        .with_context(|| format!("サブプロセス起動失敗: {}", file.display()))?;
    if !output.status.success() {
        let stderr = String::from_utf8_lossy(&output.stderr);
        let signal_info = signal_of(&output.status);
        anyhow::bail!(
            "exit={:?} signal={:?}: {}",
            output.status.code(),
            signal_info,
            stderr.lines().last().unwrap_or("(no stderr)")
        );
    }
    let payload: serde_json::Value =
        serde_json::from_slice(&output.stdout).with_context(|| "JSON parse 失敗")?;
    let ast: cclint_ast::OwnedNode = serde_json::from_value(payload["ast"].clone())?;
    let diags: Vec<Diagnostic> = serde_json::from_value(payload["diags"].clone())?;
    Ok((ast, diags))
}

fn run() -> Result<ExitCode> {
    let args = Args::parse();

    // 内部モード: 1 ファイル parse だけして JSON を吐く．SEGV 隔離用．
    if let Some(path) = &args.internal_parse {
        let cfg = Config::load(&args.config).unwrap_or_default();
        let session = Session::new()?;
        let (ast, diags) = session.parse_file(path, &cfg.cpp_standard, &[])?;
        let payload = serde_json::json!({ "ast": ast, "diags": diags });
        println!("{}", serde_json::to_string(&payload)?);
        return Ok(ExitCode::SUCCESS);
    }

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

    let mut all: Vec<Diagnostic> = Vec::new();

    // フェーズ 1: 全ファイルを parse し，AST を保持．
    // libclang が SEGV することがあるので，デフォルトでサブプロセス隔離して
    // parse する (--no-subprocess で同一プロセスにフォールバック)．
    let exe = std::env::current_exe().context("自プロセスの実行ファイル特定失敗")?;
    let mut parsed: Vec<(std::path::PathBuf, cclint_ast::OwnedNode)> = Vec::new();

    let session_inproc = if args.no_subprocess {
        Some(Session::new()?)
    } else {
        None
    };

    for f in &files {
        if args.verbose {
            use std::io::Write;
            let _ = writeln!(std::io::stderr(), "==> parse: {}", f.display());
            let _ = std::io::stderr().flush();
        }
        let result: Result<(cclint_ast::OwnedNode, Vec<Diagnostic>)> =
            if let Some(s) = &session_inproc {
                s.parse_file(f, &cfg.cpp_standard, &[])
            } else {
                parse_in_subprocess(&exe, f, &args.config)
            };
        match result {
            Ok((ast, mut diags)) => {
                let has_fatal = diags
                    .iter()
                    .any(|d| d.severity == Severity::Error && d.rule == "clang");
                all.append(&mut diags);
                if has_fatal {
                    all.push(Diagnostic::new(
                        "cclint",
                        Severity::Warning,
                        format!(
                            "clang の致命的エラーがあるため `{}` のルール実行をスキップします",
                            f.display()
                        ),
                    ));
                    continue;
                }
                engine.add_project_root(f, &ast);
                parsed.push((f.clone(), ast));
            }
            Err(e) => {
                all.push(Diagnostic::new(
                    "cclint",
                    Severity::Warning,
                    format!("parse 異常終了 (SEGV 等): {}: {e}", f.display()),
                ));
            }
        }
    }

    // フェーズ 2: 各 AST に対してルールを実行する．
    // プロジェクトインデックスは Lua から `cclint.project_*` で参照可能．
    for (_, ast) in &parsed {
        let mut rule_diags = engine.run(ast)?;
        all.append(&mut rule_diags);
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
    Ok(if fail {
        ExitCode::from(1)
    } else {
        ExitCode::SUCCESS
    })
}
