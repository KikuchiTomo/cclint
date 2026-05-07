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
use cclint_ast::{NodeRef, OwnedNode, ProjectIndex};
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
    project: Arc<Mutex<ProjectIndex>>,
}

impl Engine {
    pub fn new() -> Result<Self> {
        let lua = Lua::new();
        let rules: Arc<Mutex<Vec<LuaRule>>> = Arc::new(Mutex::new(Vec::new()));
        let diagnostics: Arc<Mutex<Vec<Diagnostic>>> = Arc::new(Mutex::new(Vec::new()));
        let current_rule: Arc<Mutex<Option<String>>> = Arc::new(Mutex::new(None));
        let project: Arc<Mutex<ProjectIndex>> = Arc::new(Mutex::new(ProjectIndex::new()));

        let cclint = lua.create_table().map_err(to_anyhow)?;

        // cclint.register
        {
            let rules_cl = Arc::clone(&rules);
            let f = lua
                .create_function(move |_, (name, def): (String, Table)| {
                    let description: String = def
                        .get::<Option<String>>("description")?
                        .unwrap_or_default();
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

        // cclint.project_definitions(usr) / cclint.project_references(usr)
        for (key, is_def) in [("project_definitions", true), ("project_references", false)] {
            let proj_cl = Arc::clone(&project);
            let f = lua
                .create_function(move |lua, usr: String| {
                    let p = proj_cl.lock().unwrap();
                    let map = if is_def {
                        &p.definitions
                    } else {
                        &p.references
                    };
                    let arr = lua.create_table()?;
                    if let Some(refs) = map.get(&usr) {
                        for (i, r) in refs.iter().enumerate() {
                            arr.set(i + 1, node_ref_to_lua(lua, r)?)?;
                        }
                    }
                    Ok(arr)
                })
                .map_err(to_anyhow)?;
            cclint.set(key, f).map_err(to_anyhow)?;
        }

        // cclint.project_files()
        {
            let proj_cl = Arc::clone(&project);
            let f = lua
                .create_function(move |lua, ()| {
                    let p = proj_cl.lock().unwrap();
                    let arr = lua.create_table()?;
                    for (i, f) in p.files.iter().enumerate() {
                        arr.set(i + 1, f.display().to_string())?;
                    }
                    Ok(arr)
                })
                .map_err(to_anyhow)?;
            cclint.set("project_files", f).map_err(to_anyhow)?;
        }

        lua.globals().set("cclint", cclint).map_err(to_anyhow)?;

        Ok(Self {
            lua,
            rules,
            diagnostics,
            current_rule,
            project,
        })
    }

    /// 全ファイルを parse した後，AST ルートからプロジェクトインデックスに追記する。
    pub fn add_project_root(&self, file: &std::path::Path, root: &OwnedNode) {
        self.project.lock().unwrap().add_root(file, root);
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
    // システムヘッダ内のノードは lint 対象外 (ユーザコードでは無いため)．
    // 例: /usr/include/.../bits/types.h の __fsid_t に命名規則を適用しない．
    if n.is_in_system_header {
        return;
    }
    out.push(n);
    for c in &n.children {
        collect(c, out);
    }
}

fn node_to_lua(lua: &Lua, n: &OwnedNode) -> mlua::Result<Table> {
    let t = lua.create_table()?;

    // 基本
    t.set("kind", n.kind.clone())?;
    t.set("name", n.name.clone())?;
    t.set("display_name", n.display_name.clone())?;
    t.set("spelling", n.spelling.clone())?;
    t.set("usr", n.usr.clone())?;
    if let Some(v) = &n.mangled_name {
        t.set("mangled_name", v.clone())?;
    }

    // 型情報
    if let Some(v) = &n.type_name {
        t.set("type_name", v.clone())?;
    }
    if let Some(v) = &n.canonical_type {
        t.set("canonical_type", v.clone())?;
    }
    if let Some(v) = &n.typedef_underlying_type {
        t.set("typedef_underlying_type", v.clone())?;
    }
    if let Some(v) = &n.result_type {
        t.set("result_type", v.clone())?;
    }
    if let Some(v) = &n.enum_underlying_type {
        t.set("enum_underlying_type", v.clone())?;
    }
    if let Some(v) = n.enum_constant_value {
        t.set("enum_constant_value", v)?;
    }
    if let Some(v) = n.bit_field_width {
        t.set("bit_field_width", v as i64)?;
    }
    if let Some(v) = n.offset_of_field {
        t.set("offset_of_field", v as i64)?;
    }

    // linkage / 可視性 / ストレージ
    if let Some(v) = &n.access {
        t.set("access", v.clone())?;
    }
    if let Some(v) = &n.linkage {
        t.set("linkage", v.clone())?;
    }
    if let Some(v) = &n.visibility {
        t.set("visibility", v.clone())?;
    }
    if let Some(v) = &n.language {
        t.set("language", v.clone())?;
    }
    if let Some(v) = &n.storage_class {
        t.set("storage_class", v.clone())?;
    }
    if let Some(v) = &n.tls_kind {
        t.set("tls_kind", v.clone())?;
    }
    if let Some(v) = &n.availability {
        t.set("availability", v.clone())?;
    }

    // predicates
    t.set("is_definition", n.is_definition)?;
    t.set("is_declaration", n.is_declaration)?;
    t.set("is_const", n.is_const)?;
    t.set("is_static", n.is_static)?;
    t.set("is_virtual", n.is_virtual)?;
    t.set("is_pure_virtual", n.is_pure_virtual)?;
    t.set("is_default_constructor", n.is_default_constructor)?;
    t.set("is_copy_constructor", n.is_copy_constructor)?;
    t.set("is_move_constructor", n.is_move_constructor)?;
    t.set("is_converting_constructor", n.is_converting_constructor)?;
    t.set("is_defaulted", n.is_defaulted)?;
    t.set("is_function_inlined", n.is_function_inlined)?;
    t.set("is_variadic", n.is_variadic)?;
    t.set("is_anonymous", n.is_anonymous)?;
    t.set("is_anonymous_record_decl", n.is_anonymous_record_decl)?;
    t.set("is_abstract_record", n.is_abstract_record)?;
    t.set("is_bit_field", n.is_bit_field)?;
    t.set("is_mutable", n.is_mutable)?;
    t.set("is_scoped", n.is_scoped)?;
    t.set("is_in_main_file", n.is_in_main_file)?;
    t.set("is_in_system_header", n.is_in_system_header)?;
    t.set("is_invalid_declaration", n.is_invalid_declaration)?;
    t.set("is_attribute", n.is_attribute)?;
    t.set("is_expression", n.is_expression)?;
    t.set("is_preprocessing", n.is_preprocessing)?;
    t.set("is_reference", n.is_reference)?;
    t.set("is_statement", n.is_statement)?;
    t.set("is_unexposed", n.is_unexposed)?;
    t.set("is_function_like_macro", n.is_function_like_macro)?;
    t.set("is_builtin_macro", n.is_builtin_macro)?;
    t.set("is_inline_namespace", n.is_inline_namespace)?;
    t.set("is_dynamic_call", n.is_dynamic_call)?;
    t.set("is_macro_expansion", n.is_macro_expansion)?;

    // 他エンティティへの参照
    if let Some(v) = &n.referenced_name {
        t.set("referenced_name", v.clone())?;
    }
    if let Some(v) = &n.referenced_usr {
        t.set("referenced_usr", v.clone())?;
    }
    if let Some(v) = &n.semantic_parent_usr {
        t.set("semantic_parent_usr", v.clone())?;
    }
    if let Some(v) = &n.lexical_parent_usr {
        t.set("lexical_parent_usr", v.clone())?;
    }
    if let Some(v) = &n.definition_usr {
        t.set("definition_usr", v.clone())?;
    }
    if let Some(v) = &n.canonical_usr {
        t.set("canonical_usr", v.clone())?;
    }
    if !n.overridden_method_usrs.is_empty() {
        let arr = lua.create_table()?;
        for (i, u) in n.overridden_method_usrs.iter().enumerate() {
            arr.set(i + 1, u.clone())?;
        }
        t.set("overridden_method_usrs", arr)?;
    }

    // include
    if let Some(f) = &n.included_file {
        t.set("included_file", f.display().to_string())?;
    }

    // コメント
    if let Some(v) = &n.comment {
        t.set("comment", v.clone())?;
    }
    if let Some(v) = &n.comment_brief {
        t.set("comment_brief", v.clone())?;
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
    if let Some(s) = &n.spelling_span {
        let st = lua.create_table()?;
        st.set("file", s.file.display().to_string())?;
        st.set("line", s.line)?;
        st.set("column", s.column)?;
        st.set("byte_start", s.byte_start as i64)?;
        st.set("byte_end", s.byte_end as i64)?;
        t.set("spelling_span", st)?;
    }
    let kids = lua.create_table()?;
    for (i, c) in n.children.iter().enumerate() {
        let child_tbl = node_to_lua(lua, c)?;
        child_tbl.set("parent", t.clone())?;
        kids.set(i + 1, child_tbl)?;
    }
    t.set("children", kids)?;
    Ok(t)
}

fn node_ref_to_lua(lua: &Lua, r: &NodeRef) -> mlua::Result<Table> {
    let t = lua.create_table()?;
    t.set("kind", r.kind.clone())?;
    t.set("name", r.name.clone())?;
    t.set("usr", r.usr.clone())?;
    let span = lua.create_table()?;
    span.set("file", r.file.display().to_string())?;
    span.set("line", r.line)?;
    span.set("column", r.column)?;
    span.set("byte_start", r.byte_start as i64)?;
    span.set("byte_end", r.byte_end as i64)?;
    t.set("span", span)?;
    if let Some(n) = &r.enclosing_function_name {
        t.set("enclosing_function_name", n.clone())?;
    }
    if let Some(u) = &r.enclosing_function_usr {
        t.set("enclosing_function_usr", u.clone())?;
    }
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
