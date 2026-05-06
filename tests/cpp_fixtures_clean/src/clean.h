#pragma once

// 全 14 ルールに違反しない健全なコード．
// stdlib ヘッダは使わない (compile_commands.json なしで素直に parse できるように)．

constexpr int kMaxItems = 100;

enum class Color { Red, Green, Blue };

class WidgetClassA {
public:
    explicit WidgetClassA(int initial);
    virtual ~WidgetClassA();

    virtual void draw() const;
    int get_count() const { return private_count_; }

    void configure(int a, int b, int c, int d, int e);

private:
    int private_count_;
    int private_value_;
};

class Plain {
public:
    Plain() = default;
    int x;
};

using IntAlias = int;
