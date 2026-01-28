#pragma once

#include <memory>
#include <string>
#include <vector>

#include "parser/ast.hpp"
#include "parser/lexer.hpp"

namespace cclint {
namespace parser {

/// 組み込みC++パーサー
/// レキサーベースの独自実装パーサー。LLVM/Clangに依存せず、
/// 基本的なC++構文（class, function, constructor, destructor, lambda,
/// template, friend, operator overloadなど）を解析できる。
class BuiltinParser {
public:
    /// コンストラクタ
    /// @param source ソースコード
    /// @param filename ファイル名
    explicit BuiltinParser(const std::string& source, const std::string& filename = "");

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

    // ヘルパー
    std::string parse_type();
    void add_error(const std::string& message);
    SourcePosition get_position() const;
};

}  // namespace parser
}  // namespace cclint
