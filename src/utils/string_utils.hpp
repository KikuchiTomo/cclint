#pragma once

#include <string>
#include <vector>

namespace utils {

// 文字列操作ユーティリティ
class StringUtils {
public:
    // 文字列を分割
    static std::vector<std::string> split(const std::string& str,
                                         char delimiter);

    // 文字列を分割（複数の区切り文字）
    static std::vector<std::string> split(const std::string& str,
                                         const std::string& delimiters);

    // 文字列を結合
    static std::string join(const std::vector<std::string>& strings,
                           const std::string& separator);

    // 前後の空白を削除
    static std::string trim(const std::string& str);

    // 前の空白を削除
    static std::string trim_left(const std::string& str);

    // 後の空白を削除
    static std::string trim_right(const std::string& str);

    // 小文字に変換
    static std::string to_lower(const std::string& str);

    // 大文字に変換
    static std::string to_upper(const std::string& str);

    // 先頭が一致するか確認
    static bool starts_with(const std::string& str, const std::string& prefix);

    // 末尾が一致するか確認
    static bool ends_with(const std::string& str, const std::string& suffix);

    // 文字列を置換
    static std::string replace(const std::string& str,
                              const std::string& from,
                              const std::string& to);

    // 全ての出現を置換
    static std::string replace_all(const std::string& str,
                                   const std::string& from,
                                   const std::string& to);

    // 文字列に含まれるか確認
    static bool contains(const std::string& str, const std::string& substr);

    // 文字列をフォーマット（簡易版）
    template<typename... Args>
    static std::string format(const std::string& fmt, Args... args);

private:
    StringUtils() = default;

    // 空白文字か判定
    static bool is_space(char c);
};

}  // namespace utils
