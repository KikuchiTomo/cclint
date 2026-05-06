//! Lua 連携層。
//!
//! Lua 側に提供する API:
//!   cclint.register(name, { description, match=fn(node), check=fn(node, ctx) })
//!   cclint.report_warn(node_or_span, message)
//!   cclint.report_error(node_or_span, message)
//!   cclint.report_info(node_or_span, message)

use std::path::Path;
use std::sync::{Arc, Mutex};

use anyhow::{Context, Result};
use cclint_ast::OwnedNode;
use cclint_diagnostic::{Diagnostic, Severity, Span};
use mlua::{Function, Lua, Table, Value};

#[derive(Clone)]
pub struct LuaRule {
    pub name: String,
    pub description: String,
    pub matcher: Option<Function>,
    pub check: Function,
    pub severity: Severity,
}

pub struct Engine {
    lua: Lua,
    rules: Arc<Mutex<Vec<LuaRule>>>,
    diagnostics: Arc<Mutex<Vec<Diagnostic>>>,
    current_rule: Arc<Mutex<Option<String>>>,
}

impl Engine {
    pub fn new() -> Result<Self> {
        let lua = Lua::new();
        let rules: Arc<Mutex<Vec<LuaRule>>> = Arc::new(Mutex::new(Vec::new()));
        let diagnostics: Arc<Mutex<Vec<Diagnostic>>> = Arc::new(Mutex::new(Vec::new()));
        let current_rule: Arc<Mutex<Option<String>>> = Arc::new(Mutex::new(None));

        let cclint = lua.create_table().map_err(to_anyhow)?;

        // cclint.register
        {
            let rules_cl = Arc::clone(&rules);
            let f = lua
                .create_function(move |_, (name, def): (String, Table)| {
                    let description: String =
                        def.get::<Option<String>>("description")?.unwrap_or_default();
                    let matcher: Option<Function> = def.get::<Option<Function>>("match")?;
                    let check: Function = def.get("check").map_err(|_| {
                        mlua::Error::RuntimeError(format!(
                            "ルール `{name}` に check 関数がありません"
                        ))
                    })?;
                    let severity_s: Option<String> = def.get::<Option<String>>("severity")?;
                    let severity = match severity_s.as_deref() {
                        Some("error") => Severity::Error,
                        Some("info") => Severity::Info,
                        Some("hint") => Severity::Hint,
                        _ => Severity::Warning,
                    };
                    rules_cl.lock().unwrap().push(LuaRule {
                        name,
                        description,
                        matcher,
                        check,
                        severity,
                    });
                    Ok(())
                })
                .map_err(to_anyhow)?;
            cclint.set("register", f).map_err(to_anyhow)?;
        }

        for (key, sev) in [
            ("report_warn", Severity::Warning),
            ("report_error", Severity::Error),
            ("report_info", Severity::Info),
            ("report_hint", Severity::Hint),
        ] {
            let diags_cl = Arc::clone(&diagnostics);
            let rule_cl = Arc::clone(&current_rule);
            let f = lua
                .create_function(move |_, (target, message): (Value, String)| {
                    let span = extract_span(&target);
                    let rule_name = rule_cl
                        .lock()
                        .unwrap()
                        .clone()
                        .unwrap_or_else(|| "anonymous".to_string());
                    let mut d = Diagnostic::new(rule_name, sev, message);
                    if let Some(s) = span {
                        d = d.with_span(s);
                    }
                    diags_cl.lock().unwrap().push(d);
                    Ok(())
                })
                .map_err(to_anyhow)?;
            cclint.set(key, f).map_err(to_anyhow)?;
        }

        lua.globals().set("cclint", cclint).map_err(to_anyhow)?;

        Ok(Self {
            lua,
            rules,
            diagnostics,
            current_rule,
        })
    }

    pub fn load_script(&self, path: &Path) -> Result<()> {
        let src = std::fs::read_to_string(path)
            .with_context(|| format!("Lua 読込失敗: {}", path.display()))?;
        self.lua
            .load(&src)
            .set_name(path.display().to_string())
            .exec()
            .map_err(to_anyhow)
            .with_context(|| format!("Lua 実行失敗: {}", path.display()))?;
        Ok(())
    }

    pub fn run(&self, root: &OwnedNode) -> Result<Vec<Diagnostic>> {
        let rules = self.rules.lock().unwrap().clone();
        let mut nodes: Vec<&OwnedNode> = Vec::new();
        collect(root, &mut nodes);
        for n in nodes {
            let node_tbl = node_to_lua(&self.lua, n).map_err(to_anyhow)?;
            for rule in &rules {
                let matched = match &rule.matcher {
                    Some(m) => m.call::<bool>(node_tbl.clone()).unwrap_or(false),
                    None => true,
                };
                if !matched {
                    continue;
                }
                *self.current_rule.lock().unwrap() = Some(rule.name.clone());
                let ctx = self.lua.create_table().map_err(to_anyhow)?;
                ctx.set("rule", rule.name.clone()).map_err(to_anyhow)?;
                if let Err(e) = rule.check.call::<()>((node_tbl.clone(), ctx)) {
                    self.diagnostics.lock().unwrap().push(Diagnostic::new(
                        rule.name.clone(),
                        Severity::Error,
                        format!("ルール実行エラー: {e}"),
                    ));
                }
                *self.current_rule.lock().unwrap() = None;
            }
        }
        let mut out = self.diagnostics.lock().unwrap();
        Ok(std::mem::take(&mut *out))
    }

    pub fn rule_count(&self) -> usize {
        self.rules.lock().unwrap().len()
    }
}

fn to_anyhow(e: mlua::Error) -> anyhow::Error {
    anyhow::anyhow!("Lua: {e}")
}

fn collect<'a>(n: &'a OwnedNode, out: &mut Vec<&'a OwnedNode>) {
    out.push(n);
    for c in &n.children {
        collect(c, out);
    }
}

fn node_to_lua(lua: &Lua, n: &OwnedNode) -> mlua::Result<Table> {
    let t = lua.create_table()?;
    t.set("kind", n.kind.clone())?;
    t.set("name", n.name.clone())?;
    t.set("display_name", n.display_name.clone())?;
    t.set("spelling", n.spelling.clone())?;
    t.set("usr", n.usr.clone())?;
    if let Some(a) = &n.access {
        t.set("access", a.clone())?;
    }
    t.set("is_definition", n.is_definition)?;
    t.set("is_const", n.is_const)?;
    t.set("is_static", n.is_static)?;
    t.set("is_virtual", n.is_virtual)?;
    t.set("is_pure_virtual", n.is_pure_virtual)?;
    if let Some(ty) = &n.type_name {
        t.set("type_name", ty.clone())?;
    }
    if let Some(s) = &n.span {
        let st = lua.create_table()?;
        st.set("file", s.file.display().to_string())?;
        st.set("line", s.line)?;
        st.set("column", s.column)?;
        st.set("byte_start", s.byte_start as i64)?;
        st.set("byte_end", s.byte_end as i64)?;
        t.set("span", st)?;
    }
    let kids = lua.create_table()?;
    for (i, c) in n.children.iter().enumerate() {
        kids.set(i + 1, node_to_lua(lua, c)?)?;
    }
    t.set("children", kids)?;
    Ok(t)
}

fn extract_span(v: &Value) -> Option<Span> {
    let t = v.as_table()?;
    if let Ok(s) = t.get::<Table>("span") {
        return table_to_span(&s);
    }
    table_to_span(t)
}

fn table_to_span(t: &Table) -> Option<Span> {
    let file: String = t.get("file").ok()?;
    let line: u32 = t.get("line").unwrap_or(0);
    let column: u32 = t.get("column").unwrap_or(0);
    let bs: i64 = t.get("byte_start").unwrap_or(0);
    let be: i64 = t.get("byte_end").unwrap_or(bs);
    Some(Span {
        file: std::path::PathBuf::from(file),
        byte_start: bs.max(0) as usize,
        byte_end: be.max(bs) as usize,
        line,
        column,
    })
}
