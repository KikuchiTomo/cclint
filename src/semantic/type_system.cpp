#include "semantic/type_system.hpp"

namespace cclint {
namespace semantic {

// ==================== Type Implementation ====================

bool Type::equals(const Type& other) const {
    if (kind_ != other.kind_) {
        return false;
    }

    // CV修飾子もチェック
    if (qualifiers_.is_const != other.qualifiers_.is_const ||
        qualifiers_.is_volatile != other.qualifiers_.is_volatile) {
        return false;
    }

    return true;
}

bool Type::convertible_to(const Type& other) const {
    // 同じ型なら変換可能
    if (equals(other)) {
        return true;
    }

    // constへの変換は常に可能
    if (kind_ == other.kind_ && !qualifiers_.is_const && other.qualifiers_.is_const) {
        return true;
    }

    // 数値型間の暗黙変換
    if ((kind_ == TypeKind::Int || kind_ == TypeKind::Float || kind_ == TypeKind::Double ||
         kind_ == TypeKind::Char || kind_ == TypeKind::Bool) &&
        (other.kind_ == TypeKind::Int || other.kind_ == TypeKind::Float ||
         other.kind_ == TypeKind::Double || other.kind_ == TypeKind::Char ||
         other.kind_ == TypeKind::Bool)) {
        return true;
    }

    return false;
}

// ==================== BuiltinType Implementation ====================

std::string BuiltinType::to_string() const {
    std::string result;

    if (qualifiers_.is_const)
        result += "const ";
    if (qualifiers_.is_volatile)
        result += "volatile ";

    switch (kind_) {
        case TypeKind::Void:
            result += "void";
            break;
        case TypeKind::Bool:
            result += "bool";
            break;
        case TypeKind::Char:
            result += "char";
            break;
        case TypeKind::Int:
            result += "int";
            break;
        case TypeKind::Float:
            result += "float";
            break;
        case TypeKind::Double:
            result += "double";
            break;
        case TypeKind::Auto:
            result += "auto";
            break;
        case TypeKind::Decltype:
            result += "decltype";
            break;
        default:
            result += "unknown";
            break;
    }

    return result;
}

// ==================== ArrayType Implementation ====================

std::string ArrayType::to_string() const {
    std::string result = element_->to_string() + "[";
    if (size_ >= 0) {
        result += std::to_string(size_);
    }
    result += "]";
    return result;
}

// ==================== FunctionType Implementation ====================

std::string FunctionType::to_string() const {
    std::string result = return_type_->to_string() + "(";

    for (size_t i = 0; i < param_types_.size(); ++i) {
        if (i > 0)
            result += ", ";
        result += param_types_[i]->to_string();
    }

    result += ")";
    return result;
}

// ==================== TypeSystem Implementation ====================

TypeSystem::TypeSystem() {
    // 組み込み型を初期化
    void_type_ = std::make_shared<BuiltinType>(TypeKind::Void);
    bool_type_ = std::make_shared<BuiltinType>(TypeKind::Bool);
    char_type_ = std::make_shared<BuiltinType>(TypeKind::Char);
    int_type_ = std::make_shared<BuiltinType>(TypeKind::Int);
    float_type_ = std::make_shared<BuiltinType>(TypeKind::Float);
    double_type_ = std::make_shared<BuiltinType>(TypeKind::Double);
}

std::shared_ptr<Type> TypeSystem::get_builtin_type(TypeKind kind) {
    switch (kind) {
        case TypeKind::Void:
            return void_type_;
        case TypeKind::Bool:
            return bool_type_;
        case TypeKind::Char:
            return char_type_;
        case TypeKind::Int:
            return int_type_;
        case TypeKind::Float:
            return float_type_;
        case TypeKind::Double:
            return double_type_;
        default:
            return std::make_shared<BuiltinType>(kind);
    }
}

std::shared_ptr<Type> TypeSystem::create_pointer_type(std::shared_ptr<Type> pointee) {
    auto ptr_type = std::make_shared<PointerType>(pointee);
    types_.push_back(ptr_type);
    return ptr_type;
}

std::shared_ptr<Type> TypeSystem::create_reference_type(std::shared_ptr<Type> referenced,
                                                        bool is_rvalue) {
    auto ref_type = std::make_shared<ReferenceType>(referenced, is_rvalue);
    types_.push_back(ref_type);
    return ref_type;
}

std::shared_ptr<Type> TypeSystem::create_array_type(std::shared_ptr<Type> element, int size) {
    auto array_type = std::make_shared<ArrayType>(element, size);
    types_.push_back(array_type);
    return array_type;
}

std::shared_ptr<Type>
TypeSystem::create_function_type(std::shared_ptr<Type> return_type,
                                 const std::vector<std::shared_ptr<Type>>& param_types) {
    auto func_type = std::make_shared<FunctionType>(return_type, param_types);
    types_.push_back(func_type);
    return func_type;
}

std::shared_ptr<ClassType> TypeSystem::create_class_type(const std::string& name) {
    auto class_type = std::make_shared<ClassType>(name);
    types_.push_back(class_type);
    return class_type;
}

std::shared_ptr<Type> TypeSystem::resolve_type(const std::string& type_name) {
    // 組み込み型のマッピング
    if (type_name == "void")
        return void_type_;
    if (type_name == "bool")
        return bool_type_;
    if (type_name == "char")
        return char_type_;
    if (type_name == "int")
        return int_type_;
    if (type_name == "float")
        return float_type_;
    if (type_name == "double")
        return double_type_;
    if (type_name == "auto")
        return get_builtin_type(TypeKind::Auto);

    // ポインタ型の検出
    if (type_name.back() == '*') {
        auto base_type_name = type_name.substr(0, type_name.length() - 1);
        // 末尾のスペースを削除
        while (!base_type_name.empty() && base_type_name.back() == ' ') {
            base_type_name.pop_back();
        }
        auto base_type = resolve_type(base_type_name);
        if (base_type) {
            return create_pointer_type(base_type);
        }
    }

    // 参照型の検出
    if (type_name.back() == '&') {
        auto base_type_name = type_name.substr(0, type_name.length() - 1);
        // 末尾のスペースを削除
        while (!base_type_name.empty() && base_type_name.back() == ' ') {
            base_type_name.pop_back();
        }
        // && (rvalue reference) のチェック
        bool is_rvalue = false;
        if (!base_type_name.empty() && base_type_name.back() == '&') {
            is_rvalue = true;
            base_type_name.pop_back();
            while (!base_type_name.empty() && base_type_name.back() == ' ') {
                base_type_name.pop_back();
            }
        }
        auto base_type = resolve_type(base_type_name);
        if (base_type) {
            return create_reference_type(base_type, is_rvalue);
        }
    }

    // その他の型（クラス型として扱う）
    return create_class_type(type_name);
}

bool TypeSystem::can_convert(std::shared_ptr<Type> from, std::shared_ptr<Type> to) const {
    if (!from || !to) {
        return false;
    }

    return from->convertible_to(*to);
}

}  // namespace semantic
}  // namespace cclint
