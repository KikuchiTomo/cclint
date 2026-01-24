#include "parser/ast.hpp"

namespace cclint {
namespace parser {

std::string ASTNode::get_type_name() const {
    switch (type) {
        case ASTNodeType::TranslationUnit:
            return "TranslationUnit";
        case ASTNodeType::Namespace:
            return "Namespace";
        case ASTNodeType::Class:
            return "Class";
        case ASTNodeType::Function:
            return "Function";
        case ASTNodeType::Method:
            return "Method";
        case ASTNodeType::Variable:
            return "Variable";
        case ASTNodeType::Parameter:
            return "Parameter";
        case ASTNodeType::Field:
            return "Field";
        case ASTNodeType::Enum:
            return "Enum";
        case ASTNodeType::EnumConstant:
            return "EnumConstant";
        case ASTNodeType::Typedef:
            return "Typedef";
        case ASTNodeType::Using:
            return "Using";
        case ASTNodeType::Template:
            return "Template";
        case ASTNodeType::Comment:
            return "Comment";
        case ASTNodeType::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace parser
} // namespace cclint
