#pragma once

#include <map>
#include <string>
#include <vector>

#include "diagnostic.hpp"

namespace diagnostic {

/// 自動修正適用クラス
class Fixer {
public:
    /// コンストラクタ
    /// @param preview_mode trueの場合、実際にファイルを変更せずにプレビューのみ表示
    explicit Fixer(bool preview_mode = false);

    /// 診断メッセージのfixit hintsを適用する
    /// @param diagnostics 診断メッセージのリスト
    /// @return 修正されたファイル数
    size_t apply_fixes(const std::vector<Diagnostic>& diagnostics);

    /// 特定のファイルに対する修正を取得
    /// @param filename ファイル名
    /// @return 修正後の内容（修正がない場合は空文字列）
    std::string get_fixed_content(const std::string& filename) const;

    /// プレビュー情報を取得
    /// @return ファイル名と修正内容のマップ
    const std::map<std::string, std::string>& get_preview() const { return fixed_files_; }

    /// 修正を実際にファイルに書き込む
    /// @return 書き込まれたファイル数
    size_t write_fixes();

private:
    bool preview_mode_;
    std::map<std::string, std::string> fixed_files_;     // ファイル名 -> 修正後の内容
    std::map<std::string, std::string> original_files_;  // ファイル名 -> 元の内容

    /// ファイルに修正を適用
    void apply_fixes_to_file(const std::string& filename, const std::vector<FixItHint>& hints);

    /// ファイル内容を読み込む
    std::string read_file(const std::string& filename);

    /// 行と列の位置からオフセット位置を計算
    size_t location_to_offset(const std::string& content, int line, int column);
};

}  // namespace diagnostic
