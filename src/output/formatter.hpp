#pragma once

#include "../diagnostic/diagnostic.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace output {

// 出力フォーマッタの基底クラス
class Formatter {
public:
    virtual ~Formatter() = default;

    // 診断メッセージをフォーマットして出力
    virtual void format(const std::vector<diagnostic::Diagnostic>& diagnostics,
                       std::ostream& out) = 0;

    // ヘッダーを出力
    virtual void write_header(std::ostream& out);

    // フッターを出力
    virtual void write_footer(const std::vector<diagnostic::Diagnostic>& diagnostics,
                             std::ostream& out);

protected:
    // 統計情報を計算
    struct Statistics {
        size_t error_count = 0;
        size_t warning_count = 0;
        size_t info_count = 0;
        size_t note_count = 0;
        size_t total_count = 0;
    };

    Statistics calculate_statistics(
        const std::vector<diagnostic::Diagnostic>& diagnostics) const;
};

}  // namespace output
