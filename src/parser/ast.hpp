#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cclint {
namespace parser {

/// ソースコード内の位置情報
struct SourcePosition {
    std::string filename;
    int line = 0;
    int column = 0;

    SourcePosition() = default;
    SourcePosition(const std::string& file, int ln, int col)
        : filename(file), line(ln), column(col) {}
};

/// ASTノードの種類
enum class ASTNodeType {
    TranslationUnit,  // ファイル全体
    Namespace,        // namespace
    Class,            // class/struct
    Function,         // 関数
    Method,           // メソッド
    Variable,         // 変数宣言
    Parameter,        // パラメータ
    Field,            // クラスメンバ変数
    Enum,             // enum
    EnumConstant,     // enum定数
    Typedef,          // typedef
    Using,            // using
    Template,         // template
    Comment,          // コメント
    Unknown
};

/// アクセス指定子
enum class AccessSpecifier {
    Public,
    Protected,
    Private,
    None  // 構造体のデフォルトpublicやnamespaceなど
};

/// ASTノードの基底クラス
class ASTNode {
public:
    ASTNodeType type;
    std::string name;
    SourcePosition position;
    std::vector<std::shared_ptr<ASTNode>> children;

    explicit ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;

    // ノードタイプを文字列で取得
    std::string get_type_name() const;
};

/// 翻訳単位（ファイル全体）
class TranslationUnitNode : public ASTNode {
public:
    TranslationUnitNode() : ASTNode(ASTNodeType::TranslationUnit) {}
};

/// 名前空間
class NamespaceNode : public ASTNode {
public:
    NamespaceNode() : ASTNode(ASTNodeType::Namespace) {}
};

/// クラス/構造体
class ClassNode : public ASTNode {
public:
    bool is_struct = false;  // struct か class か
    std::vector<std::string> base_classes;

    ClassNode() : ASTNode(ASTNodeType::Class) {}
};

/// 関数/メソッド
class FunctionNode : public ASTNode {
public:
    std::string return_type;
    std::vector<std::string> parameter_types;
    std::vector<std::string> parameter_names;
    bool is_const = false;
    bool is_static = false;
    bool is_virtual = false;
    bool is_override = false;
    bool is_final = false;
    AccessSpecifier access = AccessSpecifier::None;
    int cyclomatic_complexity = 1;  // サイクロマティック複雑度

    FunctionNode() : ASTNode(ASTNodeType::Function) {}
};

/// 変数宣言
class VariableNode : public ASTNode {
public:
    std::string type_name;
    bool is_const = false;
    bool is_static = false;
    bool is_constexpr = false;
    AccessSpecifier access = AccessSpecifier::None;

    VariableNode() : ASTNode(ASTNodeType::Variable) {}
};

/// フィールド（クラスメンバ変数）
class FieldNode : public ASTNode {
public:
    std::string type_name;
    bool is_const = false;
    bool is_static = false;
    bool is_mutable = false;
    AccessSpecifier access = AccessSpecifier::None;

    FieldNode() : ASTNode(ASTNodeType::Field) {}
};

/// Enum
class EnumNode : public ASTNode {
public:
    bool is_class = false;  // enum class かどうか
    std::string underlying_type;

    EnumNode() : ASTNode(ASTNodeType::Enum) {}
};

/// Enum定数
class EnumConstantNode : public ASTNode {
public:
    std::string value;

    EnumConstantNode() : ASTNode(ASTNodeType::EnumConstant) {}
};

/// Typedef
class TypedefNode : public ASTNode {
public:
    std::string old_name;
    std::string new_name;

    TypedefNode() : ASTNode(ASTNodeType::Typedef) {}
};

/// Using
class UsingNode : public ASTNode {
public:
    std::string target;

    UsingNode() : ASTNode(ASTNodeType::Using) {}
};

/// コメント
class CommentNode : public ASTNode {
public:
    std::string content;
    bool is_line_comment = true;  // true: //, false: /* */

    CommentNode() : ASTNode(ASTNodeType::Comment) {}
};

} // namespace parser
} // namespace cclint
