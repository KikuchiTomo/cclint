use std::path::PathBuf;
use std::process::ExitCode;

use anyhow::{Context, Result};
use clap::Parser;

use cclint_ast::{CompilationDatabase, Session};
use cclint_config::Config;
use cclint_diagnostic::{Diagnostic, Severity};
use cclint_script::Engine;

mod suppress;

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

    /// 進捗詳細を表示する (どのファイルを parse 中か stderr に出す)
    #[arg(short = 'v', long)]
    verbose: bool,

    /// プログレスバーを抑制する
    #[arg(short = 'q', long)]
    quiet: bool,

    /// 内部用: 1 ファイルだけ parse し AST+診断を JSON で stdout に書き出す。
    /// 親プロセスが各ファイルをサブプロセスで parse することで，libclang が
    /// SEGV してもプロジェクト全体の lint が止まらないようにする．
    #[arg(long = "internal-parse", hide = true)]
    internal_parse: Option<PathBuf>,

    /// 内部用: --internal-parse 時に追加するコンパイラ引数 (繰り返し可)
    #[arg(long = "internal-extra-arg", hide = true, allow_hyphen_values = true)]
    internal_extra_arg: Vec<String>,

    /// サブプロセス隔離を無効化 (デバッグ用)．通常 ON．
    #[arg(long, default_value_t = false)]
    no_subprocess: bool,

    /// compile_commands.json のパスを明示指定する (繰り返し可)．
    /// 環境変数 CCLINT_COMPILE_COMMANDS にコロン区切りで列挙可能．
    #[arg(
        long = "compile-commands",
        env = "CCLINT_COMPILE_COMMANDS",
        value_delimiter = ':'
    )]
    compile_commands_paths: Vec<PathBuf>,
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

/// .cclint.toml の [cdb] と CLI 引数を統合して compile_commands.json を解決する．
/// 複数ヒットすれば全部マージする．
fn resolve_compile_db(
    args: &Args,
    cfg: &Config,
    root: &std::path::Path,
) -> Option<CompilationDatabase> {
    if !cfg.cdb.enabled {
        return None;
    }

    let mut json_paths: Vec<std::path::PathBuf> = Vec::new();

    // 1. CLI --compile-commands (繰り返し可) と環境変数
    for p in &args.compile_commands_paths {
        json_paths.push(absolutize(p, root));
    }

    // 2. [cdb].path / paths
    if let Some(p) = &cfg.cdb.path {
        json_paths.push(absolutize(p, root));
    }
    for p in &cfg.cdb.paths {
        // ディレクトリ指定なら compile_commands.json を append
        let abs = absolutize(p, root);
        if abs.is_dir() {
            json_paths.push(abs.join("compile_commands.json"));
        } else {
            json_paths.push(abs);
        }
    }

    // 旧形式
    if json_paths.is_empty() {
        if let Some(p) = &cfg.compile_commands {
            json_paths.push(absolutize(p, root));
        }
    }

    // 3. 明示パスが無ければ自動検出: 標準的なディレクトリ候補 + ツリー walk
    if json_paths.is_empty() {
        let dirs: Vec<std::path::PathBuf> = if !cfg.cdb.search_paths.is_empty() {
            cfg.cdb
                .search_paths
                .iter()
                .map(|p| absolutize(p, root))
                .collect()
        } else {
            default_cdb_dirs(root, cfg.cdb.search_parents)
        };
        for d in dirs {
            let cand = d.join("compile_commands.json");
            if cand.is_file() {
                json_paths.push(cand);
            }
        }
        // ツリー walk による検出 (モジュール毎に compile_commands.json が散らばっているケース)
        if cfg.cdb.walk_depth > 0 {
            for found in walk_compile_commands(root, cfg.cdb.walk_depth) {
                if !json_paths.contains(&found) {
                    json_paths.push(found);
                }
            }
        }
    }

    // 重複除去
    json_paths.sort();
    json_paths.dedup();
    let json_paths: Vec<std::path::PathBuf> =
        json_paths.into_iter().filter(|p| p.is_file()).collect();

    if json_paths.is_empty() {
        return None;
    }

    for p in &json_paths {
        eprintln!("==> compile_commands: {}", p.display());
    }
    match CompilationDatabase::from_files(&json_paths) {
        Ok(db) => {
            eprintln!(
                "==> compile_commands: {} 件のエントリを統合",
                db.entry_count()
            );
            Some(db)
        }
        Err(e) => {
            eprintln!("warning: compile_commands.json 読込失敗: {e}");
            None
        }
    }
}

fn absolutize(p: &std::path::Path, root: &std::path::Path) -> std::path::PathBuf {
    if p.is_absolute() {
        p.to_path_buf()
    } else {
        root.join(p)
    }
}

/// `root` 以下を walk して compile_commands.json を見つけて返す．
/// build/target/.git などは除外．
fn walk_compile_commands(root: &std::path::Path, max_depth: u32) -> Vec<std::path::PathBuf> {
    let mut out = Vec::new();
    let walker = walkdir::WalkDir::new(root)
        .max_depth(max_depth as usize)
        .follow_links(false)
        .into_iter()
        .filter_entry(|e| {
            // ノイズになる定番ディレクトリをスキップ
            if !e.file_type().is_dir() {
                return true;
            }
            let name = e.file_name().to_string_lossy();
            !matches!(
                name.as_ref(),
                ".git"
                    | ".hg"
                    | ".svn"
                    | "node_modules"
                    | "target"
                    | ".cclint_cache"
                    | ".vscode"
                    | ".idea"
            )
        });
    for entry in walker.flatten() {
        if entry.file_name() == "compile_commands.json" && entry.file_type().is_file() {
            out.push(entry.path().to_path_buf());
        }
    }
    out
}

fn default_cdb_dirs(root: &std::path::Path, parents: u32) -> Vec<std::path::PathBuf> {
    let subs = [
        ".",
        "build",
        "build-debug",
        "build-release",
        "build-Debug",
        "build-Release",
        "cmake-build-debug",
        "cmake-build-release",
        "out",
        "out/Default",
        "target",
    ];
    let mut dirs = Vec::new();
    let mut cur: Option<&std::path::Path> = Some(root);
    let mut depth = 0;
    while let Some(d) = cur {
        for s in subs {
            dirs.push(d.join(s));
        }
        if depth >= parents {
            break;
        }
        cur = d.parent();
        depth += 1;
    }
    dirs
}

/// AST を walk し InclusionDirective ノードの included_file をすべて集める．
fn collect_inclusions(node: &cclint_ast::OwnedNode, out: &mut Vec<std::path::PathBuf>) {
    if node.kind == "InclusionDirective" {
        if let Some(f) = &node.included_file {
            out.push(f.clone());
        }
    }
    for c in &node.children {
        collect_inclusions(c, out);
    }
}

fn atty_stderr() -> bool {
    use std::io::IsTerminal;
    std::io::stderr().is_terminal()
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
    extra_args: &[String],
) -> Result<(cclint_ast::OwnedNode, Vec<Diagnostic>)> {
    let mut cmd = std::process::Command::new(exe);
    cmd.arg("--internal-parse").arg(file).arg("-c").arg(config);
    for a in extra_args {
        cmd.arg("--internal-extra-arg").arg(a);
    }
    let output = cmd
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
        let (ast, diags) = session.parse_file(path, &cfg.cpp_standard, &args.internal_extra_arg)?;
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

    let mut compile_db: Option<CompilationDatabase> = resolve_compile_db(&args, &cfg, &root);
    if compile_db.is_none() && cfg.extra_args.is_empty() && !files.is_empty() {
        eprintln!(
            "cclint: ヒント: compile_commands.json が見つかりません． \
             --compile-commands <path>，[cdb] セクション，または extra_args で \
             include パス等を指定してください．"
        );
    }

    // libclang が parse 中に SEGV することがあるので，デフォルトで
    // サブプロセス隔離する．--no-subprocess は同一プロセス parse へ戻す．
    let exe = std::env::current_exe().context("自プロセスの実行ファイル特定失敗")?;
    let mut parsed: Vec<(std::path::PathBuf, cclint_ast::OwnedNode)> = Vec::new();

    let session_inproc = if args.no_subprocess {
        Some(Session::new()?)
    } else {
        None
    };

    // 直接 compile_commands.json にエントリがある cpp を先に parse して，
    // include している header → source の対応を compile_db に書き戻す．
    // これにより後段でヘッダを parse するとき ヘッダを include している
    // ソースの引数が使える (clangd HeaderIncluderCache 相当)．
    if let Some(db) = compile_db.as_mut() {
        let direct_sources: Vec<&std::path::PathBuf> = files
            .iter()
            .filter(|f| {
                let canon = f.canonicalize().unwrap_or((*f).clone());
                db.has_direct_entry(&canon)
            })
            .collect();
        if !direct_sources.is_empty() && args.verbose {
            eprintln!(
                "==> 事前 parse: 直接エントリ {} 件から include 関係を抽出",
                direct_sources.len()
            );
        }
        for f in &direct_sources {
            let extra = db.arguments_for(f).unwrap_or_default();
            let res = if let Some(s) = &session_inproc {
                s.parse_file(f, &cfg.cpp_standard, &extra)
            } else {
                parse_in_subprocess(&exe, f, &args.config, &extra)
            };
            if let Ok((ast, _)) = res {
                let mut headers: Vec<std::path::PathBuf> = Vec::new();
                collect_inclusions(&ast, &mut headers);
                db.record_includes(f, &headers);
                // 既に AST を持ってるので保持して二度 parse しないようにする
                parsed.push((f.to_path_buf(), ast));
            }
        }
    }

    // プログレスバー: stderr が TTY かつ --quiet でないとき表示．
    let progress = if !args.quiet && !args.verbose && atty_stderr() {
        let pb = indicatif::ProgressBar::new(files.len() as u64);
        pb.set_style(
            indicatif::ProgressStyle::with_template(
                "{spinner:.green} [{elapsed_precise}] [{bar:30.cyan/blue}] {pos}/{len} {wide_msg}",
            )
            .unwrap()
            .progress_chars("=>-"),
        );
        Some(pb)
    } else {
        None
    };

    let already_parsed: std::collections::HashSet<std::path::PathBuf> =
        parsed.iter().map(|(p, _)| p.clone()).collect();
    for f in &files {
        if already_parsed.contains(f) {
            if let Some(pb) = &progress {
                pb.inc(1);
            }
            continue;
        }
        if args.verbose {
            use std::io::Write;
            let _ = writeln!(std::io::stderr(), "==> parse: {}", f.display());
            let _ = std::io::stderr().flush();
        }
        if let Some(pb) = &progress {
            let name = f
                .file_name()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_default();
            pb.set_message(name);
        }
        // compile_commands.json から該当ファイルの引数を引く．
        // 引けなかったら .cclint.toml の extra_args を fallback で使う．
        let extra: Vec<String> = compile_db
            .as_ref()
            .and_then(|db| db.arguments_for(f))
            .unwrap_or_else(|| cfg.extra_args.clone());
        let result: Result<(cclint_ast::OwnedNode, Vec<Diagnostic>)> =
            if let Some(s) = &session_inproc {
                s.parse_file(f, &cfg.cpp_standard, &extra)
            } else {
                parse_in_subprocess(&exe, f, &args.config, &extra)
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
        if let Some(pb) = &progress {
            pb.inc(1);
        }
    }
    if let Some(pb) = progress {
        pb.finish_with_message("parse 完了");
    }

    // ルールを各 AST に流す．Lua からは cclint.project_* で他 TU の情報も引ける．
    for (_, ast) in &parsed {
        let mut rule_diags = engine.run(ast)?;
        all.append(&mut rule_diags);
    }

    // suppression を適用 (インラインコメント + 設定)
    let mut filter = suppress::SuppressionFilter::new(&cfg.suppressions);
    all.retain(|d| !filter.is_suppressed(d));

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
