#pragma once

#include <memory>
#include <string>
#include <vector>

#include "parser/ast.hpp"
#include "parser/enhanced_lexer.hpp"
#include "parser/preprocessor.hpp"
#include "parser/token_types_enhanced.hpp"

namespace cclint {
namespace parser {

/// 組み込みC++パーサー（Enhanced版）
/// EnhancedLexerとPreprocessorを使用した独自実装パーサー。
/// LLVM/Clangに依存せず、基本的なC++構文（class, function, constructor,
/// destructor, lambda, template, friend, operator overloadなど）を解析できる。
class BuiltinParser {
public:
    /// コンストラクタ
    /// @param source ソースコード
    /// @param filename ファイル名
    /// @param use_preprocessor プリプロセッサを使用するか（デフォルト: true）
    explicit BuiltinParser(const std::string& source, const std::string& filename = "",
                           bool use_preprocessor = true);

    /// ASTを構築
    /// @return 翻訳単位（ファイル全体）のAST
    std::shared_ptr<TranslationUnitNode> parse();

    /// エラーメッセージを取得
    const std::vector<std::string>& get_errors() const { return errors_; }

    /// パースが成功したか
    bool has_errors() const { return !errors_.empty(); }

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::string filename_;
    std::vector<std::string> errors_;

    // トークン操作
    const Token& current_token() const;
    const Token& peek_token(int offset = 1) const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token advance();
    Token expect(TokenType type, const std::string& message);

    // パース関数
    void parse_toplevel(TranslationUnitNode& root);
    std::shared_ptr<ASTNode> parse_namespace();
    std::shared_ptr<ASTNode> parse_class_or_struct();
    std::shared_ptr<ASTNode> parse_enum();
    std::shared_ptr<ASTNode> parse_function_or_variable();
    std::shared_ptr<ASTNode> parse_typedef();
    std::shared_ptr<ASTNode> parse_using();
    void skip_to_semicolon();
    void skip_braces();
    AccessSpecifier current_access_ = AccessSpecifier::None;

    // 式パーサー (Expression Parsing)
    std::shared_ptr<ASTNode> parse_expression();
    std::shared_ptr<ASTNode> parse_assignment_expression();
    std::shared_ptr<ASTNode> parse_conditional_expression();
    std::shared_ptr<ASTNode> parse_logical_or_expression();
    std::shared_ptr<ASTNode> parse_logical_and_expression();
    std::shared_ptr<ASTNode> parse_equality_expression();
    std::shared_ptr<ASTNode> parse_relational_expression();
    std::shared_ptr<ASTNode> parse_additive_expression();
    std::shared_ptr<ASTNode> parse_multiplicative_expression();
    std::shared_ptr<ASTNode> parse_unary_expression();
    std::shared_ptr<ASTNode> parse_postfix_expression();
    std::shared_ptr<ASTNode> parse_primary_expression();

    // 文パーサー (Statement Parsing)
    std::shared_ptr<ASTNode> parse_statement();
    std::shared_ptr<ASTNode> parse_compound_statement();
    std::shared_ptr<ASTNode> parse_if_statement();
    std::shared_ptr<ASTNode> parse_switch_statement();
    std::shared_ptr<ASTNode> parse_for_statement();
    std::shared_ptr<ASTNode> parse_while_statement();
    std::shared_ptr<ASTNode> parse_do_while_statement();
    std::shared_ptr<ASTNode> parse_return_statement();
    std::shared_ptr<ASTNode> parse_try_statement();
    std::shared_ptr<ASTNode> parse_declaration_statement();

    // ヘルパー
    std::string parse_type();
    void add_error(const std::string& message);
    SourcePosition get_position() const;
};

}  // namespace parser
}  // namespace cclint
