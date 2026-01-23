#pragma once

#include "formatter.hpp"

#include <memory>
#include <string>

namespace output {

// フォーマッタのファクトリークラス
class FormatterFactory {
public:
    // フォーマット名からフォーマッタを作成
    static std::unique_ptr<Formatter> create(const std::string& format_name);

    // サポートされているフォーマット名のリストを取得
    static std::vector<std::string> get_supported_formats();

    // フォーマット名が有効か判定
    static bool is_valid_format(const std::string& format_name);
};

}  // namespace output
