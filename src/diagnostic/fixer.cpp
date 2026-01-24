#include "fixer.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace diagnostic {

Fixer::Fixer(bool preview_mode) : preview_mode_(preview_mode) {}

size_t Fixer::apply_fixes(const std::vector<Diagnostic>& diagnostics) {
    // ファイルごとにfixitヒントをグループ化
    std::map<std::string, std::vector<FixItHint>> fixes_by_file;

    for (const auto& diag : diagnostics) {
        if (!diag.fix_hints.empty()) {
            const std::string& filename = diag.location.filename;
            for (const auto& hint : diag.fix_hints) {
                if (hint.range.is_valid()) {
                    fixes_by_file[filename].push_back(hint);
                }
            }
        }
    }

    // 各ファイルに対して修正を適用
    for (const auto& [filename, hints] : fixes_by_file) {
        apply_fixes_to_file(filename, hints);
    }

    return fixed_files_.size();
}

void Fixer::apply_fixes_to_file(const std::string& filename,
                                 const std::vector<FixItHint>& hints) {
    if (hints.empty()) {
        return;
    }

    // ファイル内容を読み込む
    std::string content = read_file(filename);
    original_files_[filename] = content;

    // fixitヒントを位置でソート（後ろから適用するため降順）
    auto sorted_hints = hints;
    std::sort(sorted_hints.begin(), sorted_hints.end(),
              [&content, this](const FixItHint& a, const FixItHint& b) {
                  size_t offset_a = location_to_offset(
                      content, a.range.begin.line, a.range.begin.column);
                  size_t offset_b = location_to_offset(
                      content, b.range.begin.line, b.range.begin.column);
                  return offset_a > offset_b;  // 降順
              });

    // 後ろから修正を適用（オフセット位置が変わらないようにするため）
    for (const auto& hint : sorted_hints) {
        size_t start_offset = location_to_offset(
            content, hint.range.begin.line, hint.range.begin.column);
        size_t end_offset = location_to_offset(
            content, hint.range.end.line, hint.range.end.column);

        if (start_offset <= end_offset && end_offset <= content.size()) {
            content.replace(start_offset, end_offset - start_offset,
                            hint.replacement_text);
        }
    }

    fixed_files_[filename] = content;

    if (preview_mode_) {
        utils::Logger::instance().info("Preview fix for: " + filename);
    } else {
        utils::Logger::instance().info("Applied fixes to: " + filename);
    }
}

std::string Fixer::read_file(const std::string& filename) {
    try {
        return utils::FileUtils::read_file(filename);
    } catch (const std::exception& e) {
        utils::Logger::instance().error("Failed to read file: " + filename +
                                        " - " + e.what());
        return "";
    }
}

size_t Fixer::location_to_offset(const std::string& content, int line,
                                  int column) {
    if (line <= 0 || column < 0) {
        return 0;
    }

    size_t offset = 0;
    int current_line = 1;

    // 指定された行まで移動
    while (current_line < line && offset < content.size()) {
        if (content[offset] == '\n') {
            current_line++;
        }
        offset++;
    }

    // 列の位置まで移動
    int current_column = 0;
    while (current_column < column && offset < content.size() &&
           content[offset] != '\n') {
        offset++;
        current_column++;
    }

    return offset;
}

std::string Fixer::get_fixed_content(const std::string& filename) const {
    auto it = fixed_files_.find(filename);
    if (it != fixed_files_.end()) {
        return it->second;
    }
    return "";
}

size_t Fixer::write_fixes() {
    if (preview_mode_) {
        utils::Logger::instance().info(
            "Preview mode: not writing changes to files");
        return 0;
    }

    size_t written_count = 0;
    for (const auto& [filename, content] : fixed_files_) {
        try {
            utils::FileUtils::write_file(filename, content);
            utils::Logger::instance().info("Wrote fixes to: " + filename);
            written_count++;
        } catch (const std::exception& e) {
            utils::Logger::instance().error("Failed to write file: " +
                                            filename + " - " + e.what());
        }
    }

    return written_count;
}

} // namespace diagnostic
