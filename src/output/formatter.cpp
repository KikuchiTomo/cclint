#include "formatter.hpp"

namespace output {

void Formatter::write_header(std::ostream& out) {
    // デフォルト実装は何もしない
    (void)out;
}

void Formatter::write_footer(const std::vector<diagnostic::Diagnostic>& diagnostics,
                             std::ostream& out) {
    // デフォルト実装は何もしない
    (void)diagnostics;
    (void)out;
}

Formatter::Statistics
Formatter::calculate_statistics(const std::vector<diagnostic::Diagnostic>& diagnostics) const {
    Statistics stats;
    stats.total_count = diagnostics.size();

    for (const auto& diag : diagnostics) {
        switch (diag.severity) {
            case diagnostic::Severity::Error:
                ++stats.error_count;
                break;
            case diagnostic::Severity::Warning:
                ++stats.warning_count;
                break;
            case diagnostic::Severity::Info:
                ++stats.info_count;
                break;
            case diagnostic::Severity::Note:
                ++stats.note_count;
                break;
        }
    }

    return stats;
}

}  // namespace output
