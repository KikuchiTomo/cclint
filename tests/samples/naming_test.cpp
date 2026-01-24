// naming_test.cpp
// アクセス指定子ごとの命名規則テスト用サンプル

#include <string>

// 正しい命名規則のクラス
class GoodExample {
public:
    // public: camelCase (正しい)
    void getValue() {}
    void processData() {}
    void setName(const std::string& name) {}

protected:
    // protected: snake_case (正しい)
    void do_something() {}
    void helper_function() {}

private:
    // private: _snake_case (正しい)
    void _internal_method() {}
    void _helper() {}

    int value_;
};

// 間違った命名規則のクラス
class BadExample {
public:
    // public: camelCaseであるべき (間違い: snake_case)
    void get_value() {}  // 警告: should use camelCase
    void process_data() {}  // 警告: should use camelCase

    // public: camelCaseであるべき (間違い: PascalCase)
    void SetName() {}  // 警告: should use camelCase

protected:
    // protected: snake_caseであるべき (間違い: camelCase)
    void doSomething() {}  // 警告: should use snake_case
    void helperFunction() {}  // 警告: should use snake_case

private:
    // private: _snake_caseであるべき (間違い: アンダースコアなし)
    void internal_method() {}  // 警告: should start with underscore

    // private: _snake_caseであるべき (間違い: camelCase)
    void helperMethod() {}  // 警告: should start with underscore and use snake_case

    int m_value;  // Hungarian notation（プロジェクトのルール次第）
};

// クラス名のテスト
class good_class_name {};  // 警告: should use PascalCase
struct BadStructName {};   // structは許容されることが多い

// 正しいクラス名
class GoodClassName {};
struct GoodStruct {};

namespace MyNamespace {
    class AnotherGoodClass {
    public:
        void doWork() {}  // OK

    private:
        void _do_internal_work() {}  // OK
    };
}
