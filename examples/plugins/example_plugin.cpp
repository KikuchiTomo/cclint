// Example cclint plugin
// This demonstrates how to create a plugin rule for cclint

#include "cclint/plugin_api.h"
#include "rules/rule_base.hpp"
#include "diagnostic/diagnostic.hpp"
#include "parser/ast.hpp"

#include <string>

namespace {

// Custom rule implementation
class ExamplePluginRule : public cclint::rules::RuleBase {
public:
    ExamplePluginRule() {
        name_ = "example-plugin-rule";
        description_ = "Example plugin rule that detects TODOs in code";
        category_ = "documentation";
    }

    void check_file(const std::string& file_path,
                   const std::string& file_content,
                   cclint::diagnostic::DiagnosticEngine& diag_engine) override {
        // Simple text-based check
        size_t line_num = 1;
        size_t pos = 0;

        while (pos < file_content.size()) {
            size_t line_end = file_content.find('\n', pos);
            if (line_end == std::string::npos) {
                line_end = file_content.size();
            }

            std::string line = file_content.substr(pos, line_end - pos);

            // Check for TODO comments
            if (line.find("TODO") != std::string::npos) {
                cclint::diagnostic::Diagnostic diag;
                diag.severity = cclint::diagnostic::Severity::Info;
                diag.rule_name = name_;
                diag.message = "TODO comment found";
                diag.location.filename = file_path;
                diag.location.line = line_num;
                diag.location.column = line.find("TODO");
                diag_engine.add_diagnostic(diag);
            }

            pos = line_end + 1;
            line_num++;
        }
    }

    void check_ast(const std::string& file_path,
                   std::shared_ptr<cclint::parser::TranslationUnitNode> ast,
                   cclint::diagnostic::DiagnosticEngine& diag_engine) override {
        // Optional: implement AST-based checks here
        (void)file_path;
        (void)ast;
        (void)diag_engine;
    }
};

} // anonymous namespace

// Plugin API implementation

extern "C" {

const char* cclint_plugin_get_name() {
    return "example-plugin";
}

const char* cclint_plugin_get_description() {
    return "Example plugin demonstrating the cclint plugin API";
}

const char* cclint_plugin_get_category() {
    return "example";
}

void* cclint_plugin_create_rule() {
    return new ExamplePluginRule();
}

void cclint_plugin_destroy_rule(void* rule) {
    delete static_cast<ExamplePluginRule*>(rule);
}

} // extern "C"
