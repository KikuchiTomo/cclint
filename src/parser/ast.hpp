#pragma once

#include <map>
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
    int indent_count = 0;    // インデント文字数
    bool uses_tabs = false;  // タブを使っているか

    SourcePosition() = default;
    SourcePosition(const std::string& file, int ln, int col)
        : filename(file), line(ln), column(col) {}
};

/// アクセス指定子（前方宣言が必要なため先に定義）
enum class AccessSpecifier {
    Public,
    Protected,
    Private,
    None  // 構造体のデフォルトpublicやnamespaceなど
};

/// 型情報の詳細
struct TypeInfo {
    std::string base_type;                   // 基本型名
    bool is_const = false;                   // const修飾
    bool is_volatile = false;                // volatile修飾
    bool is_pointer = false;                 // ポインタ
    bool is_reference = false;               // 参照（&）
    bool is_rvalue_reference = false;        // 右辺値参照（&&）
    int pointer_level = 0;                   // ポインタの深さ（int**なら2）
    std::vector<int> array_dimensions;       // 配列の次元
    bool is_template = false;                // テンプレート型
    std::vector<std::string> template_args;  // テンプレート引数
};

/// 継承情報
struct InheritanceInfo {
    std::string base_class_name;
    AccessSpecifier access = AccessSpecifier::Public;  // 継承のアクセス指定子
    bool is_virtual = false;                           // virtual継承
};

/// 属性情報（C++11/14/17/20 attributes）
struct AttributeInfo {
    std::string name;                    // nodiscard, deprecated, maybe_unused など
    std::vector<std::string> arguments;  // 属性の引数
};

/// テンプレートパラメータ情報
struct TemplateParameter {
    enum class Kind { Type, NonType, Template };
    Kind kind;
    std::string name;
    std::string default_value;
    bool is_variadic = false;  // パラメータパック
};

/// ASTノードの種類
enum class ASTNodeType {
    TranslationUnit,         // ファイル全体
    Namespace,               // namespace
    Class,                   // class/struct/union
    Function,                // 関数
    Method,                  // メソッド
    Constructor,             // コンストラクタ
    Destructor,              // デストラクタ
    Variable,                // 変数宣言
    Parameter,               // パラメータ
    Field,                   // クラスメンバ変数
    Enum,                    // enum
    EnumConstant,            // enum定数
    Typedef,                 // typedef
    Using,                   // using（型エイリアス or using namespace）
    Template,                // template
    TemplateSpecialization,  // テンプレート特殊化
    Comment,                 // コメント
    IfStatement,             // if文
    SwitchStatement,         // switch文
    LoopStatement,           // for/while/do-while文
    TryStatement,            // try-catch文
    ReturnStatement,         // return文
    BreakStatement,          // break文
    ContinueStatement,       // continue文
    Macro,                   // マクロ定義
    CallExpression,          // 関数呼び出し
    Lambda,                  // ラムダ式
    Operator,                // 演算子オーバーロード
    Friend,                  // friend宣言
    StaticAssert,            // static_assert
    Attribute,               // [[attribute]]
    Unknown
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

/// クラス/構造体/共用体
class ClassNode : public ASTNode {
public:
    bool is_struct = false;    // struct か class か
    bool is_union = false;     // union か
    bool is_template = false;  // テンプレートクラスか
    bool is_abstract = false;  // 純粋仮想関数を持つか
    bool is_final = false;     // final指定されているか

    std::vector<InheritanceInfo> base_classes;     // 継承情報（詳細版）
    std::vector<std::string> template_parameters;  // テンプレートパラメータ
    std::vector<AttributeInfo> attributes;         // 属性
    std::vector<std::string> friend_classes;       // friend宣言されたクラス
    std::vector<std::string> friend_functions;     // friend宣言された関数

    ClassNode() : ASTNode(ASTNodeType::Class) {}
};

/// 関数/メソッド
class FunctionNode : public ASTNode {
public:
    std::string return_type;
    TypeInfo return_type_info;  // 詳細な戻り値型情報

    std::vector<std::string> parameter_types;
    std::vector<std::string> parameter_names;
    std::vector<TypeInfo> parameter_type_infos;  // 詳細なパラメータ型情報

    bool is_const = false;
    bool is_static = false;
    bool is_virtual = false;
    bool is_override = false;
    bool is_final = false;
    bool is_pure_virtual = false;  // 純粋仮想関数（= 0）
    bool is_default = false;       // = default
    bool is_delete = false;        // = delete
    bool is_explicit = false;      // explicit指定（コンストラクタ）
    bool is_noexcept = false;      // noexcept指定
    bool is_constexpr = false;     // constexpr関数
    bool is_consteval = false;     // consteval関数（C++20）
    bool is_inline = false;        // inline関数
    bool is_template = false;      // テンプレート関数

    AccessSpecifier access = AccessSpecifier::None;
    int cyclomatic_complexity = 1;  // サイクロマティック複雑度

    std::vector<AttributeInfo> attributes;  // 属性（[[nodiscard]]など）
    std::vector<std::string> template_parameters;  // テンプレートパラメータ
    std::vector<std::string> called_functions;  // この関数が呼び出す関数のリスト
    std::vector<std::string> called_by_functions;  // この関数を呼び出す関数のリスト
    std::string exception_spec;  // 例外指定（throw()など、非推奨だが）

    FunctionNode() : ASTNode(ASTNodeType::Function) {}
};

/// 変数宣言
class VariableNode : public ASTNode {
public:
    std::string type_name;
    TypeInfo type_info;  // 詳細な型情報

    bool is_const = false;
    bool is_static = false;
    bool is_constexpr = false;
    bool is_consteval = false;  // consteval変数（C++20）
    bool is_volatile = false;
    bool is_extern = false;        // extern指定
    bool is_thread_local = false;  // thread_local指定

    std::string initializer;  // 初期化式（文字列）
    bool has_initializer = false;

    AccessSpecifier access = AccessSpecifier::None;
    std::vector<AttributeInfo> attributes;  // 属性

    VariableNode() : ASTNode(ASTNodeType::Variable) {}
};

/// フィールド（クラスメンバ変数）
class FieldNode : public ASTNode {
public:
    std::string type_name;
    TypeInfo type_info;  // 詳細な型情報

    bool is_const = false;
    bool is_static = false;
    bool is_mutable = false;
    bool is_constexpr = false;
    bool is_volatile = false;

    std::string initializer;  // メンバ初期化子
    bool has_initializer = false;
    bool has_default_member_initializer = false;  // int x = 5; のような初期化

    AccessSpecifier access = AccessSpecifier::None;
    std::vector<AttributeInfo> attributes;  // 属性

    FieldNode() : ASTNode(ASTNodeType::Field) {}
};

/// Enum
class EnumNode : public ASTNode {
public:
    bool is_class = false;                  // enum class かどうか
    bool is_scoped = false;                 // スコープ付きenum
    std::string underlying_type;            // 基底型（: int など）
    std::vector<AttributeInfo> attributes;  // 属性

    EnumNode() : ASTNode(ASTNodeType::Enum) {}
};

/// Enum定数
class EnumConstantNode : public ASTNode {
public:
    std::string value;                      // 値（文字列表現）
    bool has_explicit_value = false;        // 明示的に値が指定されているか
    std::vector<AttributeInfo> attributes;  // 属性

    EnumConstantNode() : ASTNode(ASTNodeType::EnumConstant) {}
};

/// Typedef
class TypedefNode : public ASTNode {
public:
    std::string old_name;                   // 元の型名
    std::string new_name;                   // 新しい型名
    TypeInfo type_info;                     // 詳細な型情報
    std::vector<AttributeInfo> attributes;  // 属性

    TypedefNode() : ASTNode(ASTNodeType::Typedef) {}
};

/// Using（型エイリアス or using namespace）
class UsingNode : public ASTNode {
public:
    enum class UsingKind { TypeAlias, Namespace, Declaration };
    UsingKind kind = UsingKind::Namespace;

    std::string target;  // 対象（namespace名 or 型名）
    std::string alias;   // エイリアス名（型エイリアスの場合）
    TypeInfo type_info;  // 型エイリアスの場合の詳細型情報

    bool is_template = false;  // テンプレートエイリアス
    std::vector<std::string> template_parameters;

    UsingNode() : ASTNode(ASTNodeType::Using) {}
};

/// コメント
class CommentNode : public ASTNode {
public:
    std::string content;
    bool is_line_comment = true;  // true: //, false: /* */

    CommentNode() : ASTNode(ASTNodeType::Comment) {}
};

/// If文
class IfStatementNode : public ASTNode {
public:
    bool has_braces = false;
    bool has_else = false;

    IfStatementNode() : ASTNode(ASTNodeType::IfStatement) {}
};

/// Switch文
class SwitchStatementNode : public ASTNode {
public:
    bool has_default = false;
    int case_count = 0;

    SwitchStatementNode() : ASTNode(ASTNodeType::SwitchStatement) {}
};

/// ループ文（for/while/do-while）
class LoopStatementNode : public ASTNode {
public:
    enum class LoopType { For, While, DoWhile };
    LoopType loop_type;
    bool has_braces = false;

    LoopStatementNode(LoopType type = LoopType::For)
        : ASTNode(ASTNodeType::LoopStatement), loop_type(type) {}
};

/// Try-Catch文
class TryStatementNode : public ASTNode {
public:
    int catch_count = 0;
    bool has_finally = false;

    TryStatementNode() : ASTNode(ASTNodeType::TryStatement) {}
};

/// マクロ定義
class MacroNode : public ASTNode {
public:
    bool is_function = false;
    std::vector<std::string> parameters;
    std::string definition;

    MacroNode() : ASTNode(ASTNodeType::Macro) {}
};

/// 関数呼び出し
class CallExpressionNode : public ASTNode {
public:
    std::string function_name;           // 呼び出される関数名
    std::string caller_function;         // 呼び出し元の関数名
    std::string caller_class;            // 呼び出し元のクラス名
    std::string caller_namespace;        // 呼び出し元のnamespace
    std::vector<std::string> arguments;  // 引数（文字列表現）
    bool is_method_call = false;         // メソッド呼び出しか
    bool is_virtual_call = false;        // 仮想関数呼び出しか
    std::string scope;                   // スコープ（global/class/namespace/function）

    CallExpressionNode() : ASTNode(ASTNodeType::CallExpression) {}
};

/// テンプレート宣言
class TemplateNode : public ASTNode {
public:
    std::vector<TemplateParameter> parameters;  // テンプレートパラメータ
    bool is_variadic = false;                   // 可変長テンプレート
    bool is_specialization = false;             // 特殊化
    bool is_partial_specialization = false;     // 部分特殊化
    std::string specialized_name;               // 特殊化対象の名前

    TemplateNode() : ASTNode(ASTNodeType::Template) {}
};

/// コンストラクタ
class ConstructorNode : public ASTNode {
public:
    std::string class_name;
    std::vector<std::string> parameter_types;
    std::vector<std::string> parameter_names;
    std::vector<TypeInfo> parameter_type_infos;

    bool is_default = false;    // = default
    bool is_delete = false;     // = delete
    bool is_explicit = false;   // explicit指定
    bool is_constexpr = false;  // constexpr
    bool is_noexcept = false;   // noexcept

    std::vector<std::string> initializer_list;  // メンバ初期化リスト
    AccessSpecifier access = AccessSpecifier::Public;
    std::vector<AttributeInfo> attributes;

    ConstructorNode() : ASTNode(ASTNodeType::Constructor) {}
};

/// デストラクタ
class DestructorNode : public ASTNode {
public:
    std::string class_name;

    bool is_virtual = false;
    bool is_default = false;   // = default
    bool is_delete = false;    // = delete
    bool is_noexcept = false;  // noexcept

    AccessSpecifier access = AccessSpecifier::Public;
    std::vector<AttributeInfo> attributes;

    DestructorNode() : ASTNode(ASTNodeType::Destructor) {}
};

/// ラムダ式
class LambdaNode : public ASTNode {
public:
    std::string capture_clause;          // キャプチャ節
    std::vector<std::string> captures;   // キャプチャされた変数
    bool captures_by_reference = false;  // 参照キャプチャ
    bool captures_by_value = false;      // 値キャプチャ

    std::string return_type;
    std::vector<std::string> parameter_types;
    std::vector<std::string> parameter_names;

    bool is_mutable = false;
    bool is_constexpr = false;
    bool is_noexcept = false;

    LambdaNode() : ASTNode(ASTNodeType::Lambda) {}
};

/// 演算子オーバーロード
class OperatorNode : public ASTNode {
public:
    std::string operator_symbol;  // +, -, *, /, ==, << など
    std::string return_type;
    std::vector<std::string> parameter_types;
    std::vector<std::string> parameter_names;

    bool is_member = false;  // メンバ関数か
    bool is_friend = false;  // friend関数か
    bool is_const = false;
    bool is_static = false;
    bool is_virtual = false;

    AccessSpecifier access = AccessSpecifier::None;
    std::vector<AttributeInfo> attributes;

    OperatorNode() : ASTNode(ASTNodeType::Operator) {}
};

/// friend宣言
class FriendNode : public ASTNode {
public:
    enum class FriendKind { Class, Function };
    FriendKind kind;

    std::string target_name;  // friend宣言された対象
    bool is_template = false;

    FriendNode() : ASTNode(ASTNodeType::Friend) {}
};

/// static_assert
class StaticAssertNode : public ASTNode {
public:
    std::string condition;  // 条件式
    std::string message;    // エラーメッセージ

    StaticAssertNode() : ASTNode(ASTNodeType::StaticAssert) {}
};

/// return文
class ReturnStatementNode : public ASTNode {
public:
    std::string return_value;  // 戻り値（文字列表現）
    bool has_value = false;

    ReturnStatementNode() : ASTNode(ASTNodeType::ReturnStatement) {}
};

/// break文
class BreakStatementNode : public ASTNode {
public:
    BreakStatementNode() : ASTNode(ASTNodeType::BreakStatement) {}
};

/// continue文
class ContinueStatementNode : public ASTNode {
public:
    ContinueStatementNode() : ASTNode(ASTNodeType::ContinueStatement) {}
};

}  // namespace parser
}  // namespace cclint
