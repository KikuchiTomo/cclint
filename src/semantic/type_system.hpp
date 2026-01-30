#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cclint {
namespace semantic {

/// 型の種類
enum class TypeKind {
    Void,
    Bool,
    Char,
    Int,
    Float,
    Double,
    Auto,             // C++11 auto
    Decltype,         // C++11 decltype
    Pointer,          // T*
    Reference,        // T&
    RValueReference,  // T&& (C++11)
    Array,            // T[]
    Function,         // ret(args...)
    Class,            // class/struct
    Enum,             // enum
    Template,         // template<...>
    Dependent,        // テンプレート依存型
    Unknown
};

/// CV修飾子
struct CVQualifiers {
    bool is_const = false;
    bool is_volatile = false;
    bool is_mutable = false;  // クラスメンバの場合
};

/// 型情報
class Type {
public:
    explicit Type(TypeKind kind) : kind_(kind) {}
    virtual ~Type() = default;

    TypeKind kind() const { return kind_; }

    /// この型が const か
    bool is_const() const { return qualifiers_.is_const; }

    /// CV修飾子を設定
    void set_qualifiers(const CVQualifiers& qualifiers) { qualifiers_ = qualifiers; }

    /// CV修飾子を取得
    const CVQualifiers& qualifiers() const { return qualifiers_; }

    /// 型名を文字列として取得
    virtual std::string to_string() const = 0;

    /// 型の等価性チェック
    virtual bool equals(const Type& other) const;

    /// 別の型に変換可能か (implicit conversion)
    virtual bool convertible_to(const Type& other) const;

protected:
    TypeKind kind_;
    CVQualifiers qualifiers_;
};

/// 組み込み型 (int, bool, etc.)
class BuiltinType : public Type {
public:
    explicit BuiltinType(TypeKind kind) : Type(kind) {}

    std::string to_string() const override;
};

/// ポインタ型
class PointerType : public Type {
public:
    explicit PointerType(std::shared_ptr<Type> pointee)
        : Type(TypeKind::Pointer), pointee_(pointee) {}

    std::shared_ptr<Type> pointee() const { return pointee_; }

    std::string to_string() const override { return pointee_->to_string() + "*"; }

private:
    std::shared_ptr<Type> pointee_;
};

/// 参照型
class ReferenceType : public Type {
public:
    explicit ReferenceType(std::shared_ptr<Type> referenced, bool is_rvalue = false)
        : Type(is_rvalue ? TypeKind::RValueReference : TypeKind::Reference),
          referenced_(referenced) {}

    std::shared_ptr<Type> referenced() const { return referenced_; }

    std::string to_string() const override {
        if (kind_ == TypeKind::RValueReference) {
            return referenced_->to_string() + "&&";
        }
        return referenced_->to_string() + "&";
    }

private:
    std::shared_ptr<Type> referenced_;
};

/// 配列型
class ArrayType : public Type {
public:
    ArrayType(std::shared_ptr<Type> element, int size = -1)
        : Type(TypeKind::Array), element_(element), size_(size) {}

    std::shared_ptr<Type> element() const { return element_; }
    int size() const { return size_; }

    std::string to_string() const override;

private:
    std::shared_ptr<Type> element_;
    int size_;  // -1 = unknown size
};

/// 関数型
class FunctionType : public Type {
public:
    FunctionType(std::shared_ptr<Type> return_type,
                 const std::vector<std::shared_ptr<Type>>& param_types)
        : Type(TypeKind::Function), return_type_(return_type), param_types_(param_types) {}

    std::shared_ptr<Type> return_type() const { return return_type_; }
    const std::vector<std::shared_ptr<Type>>& param_types() const { return param_types_; }

    std::string to_string() const override;

private:
    std::shared_ptr<Type> return_type_;
    std::vector<std::shared_ptr<Type>> param_types_;
};

/// クラス型
class ClassType : public Type {
public:
    explicit ClassType(const std::string& name) : Type(TypeKind::Class), name_(name) {}

    const std::string& name() const { return name_; }

    std::string to_string() const override { return name_; }

    /// 基底クラスを追加
    void add_base(std::shared_ptr<ClassType> base) { bases_.push_back(base); }

    const std::vector<std::shared_ptr<ClassType>>& bases() const { return bases_; }

private:
    std::string name_;
    std::vector<std::shared_ptr<ClassType>> bases_;
};

/// 型システム (型の作成と管理)
class TypeSystem {
public:
    TypeSystem();

    /// 組み込み型を取得
    std::shared_ptr<Type> get_builtin_type(TypeKind kind);

    /// ポインタ型を作成
    std::shared_ptr<Type> create_pointer_type(std::shared_ptr<Type> pointee);

    /// 参照型を作成
    std::shared_ptr<Type> create_reference_type(std::shared_ptr<Type> referenced,
                                                bool is_rvalue = false);

    /// 配列型を作成
    std::shared_ptr<Type> create_array_type(std::shared_ptr<Type> element, int size = -1);

    /// 関数型を作成
    std::shared_ptr<Type>
    create_function_type(std::shared_ptr<Type> return_type,
                         const std::vector<std::shared_ptr<Type>>& param_types);

    /// クラス型を作成
    std::shared_ptr<ClassType> create_class_type(const std::string& name);

    /// 型名から型を解決
    std::shared_ptr<Type> resolve_type(const std::string& type_name);

    /// 型変換が可能かチェック
    bool can_convert(std::shared_ptr<Type> from, std::shared_ptr<Type> to) const;

private:
    // 組み込み型のキャッシュ
    std::shared_ptr<Type> void_type_;
    std::shared_ptr<Type> bool_type_;
    std::shared_ptr<Type> char_type_;
    std::shared_ptr<Type> int_type_;
    std::shared_ptr<Type> float_type_;
    std::shared_ptr<Type> double_type_;

    // 作成した型のキャッシュ
    std::vector<std::shared_ptr<Type>> types_;
};

}  // namespace semantic
}  // namespace cclint
