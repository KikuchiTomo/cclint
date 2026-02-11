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
        case ASTNodeType::IfStatement:
            return "IfStatement";
        case ASTNodeType::SwitchStatement:
            return "SwitchStatement";
        case ASTNodeType::LoopStatement:
            return "LoopStatement";
        case ASTNodeType::TryStatement:
            return "TryStatement";
        case ASTNodeType::Macro:
            return "Macro";
        case ASTNodeType::CallExpression:
            return "CallExpression";
        case ASTNodeType::Lambda:
            return "Lambda";
        case ASTNodeType::Operator:
            return "Operator";
        case ASTNodeType::Friend:
            return "Friend";
        case ASTNodeType::StaticAssert:
            return "StaticAssert";
        case ASTNodeType::Attribute:
            return "Attribute";
        case ASTNodeType::Constructor:
            return "Constructor";
        case ASTNodeType::Destructor:
            return "Destructor";
        case ASTNodeType::ReturnStatement:
            return "ReturnStatement";
        case ASTNodeType::BreakStatement:
            return "BreakStatement";
        case ASTNodeType::ContinueStatement:
            return "ContinueStatement";
        case ASTNodeType::CoAwaitExpression:
            return "CoAwaitExpression";
        case ASTNodeType::CoYieldExpression:
            return "CoYieldExpression";
        case ASTNodeType::CoReturnStatement:
            return "CoReturnStatement";
        case ASTNodeType::ModuleDeclaration:
            return "ModuleDeclaration";
        case ASTNodeType::ImportDeclaration:
            return "ImportDeclaration";
        case ASTNodeType::ExportDeclaration:
            return "ExportDeclaration";
        case ASTNodeType::TemplateSpecialization:
            return "TemplateSpecialization";
        case ASTNodeType::Unknown:
        default:
            return "Unknown";
    }
}

}  // namespace parser
}  // namespace cclint
