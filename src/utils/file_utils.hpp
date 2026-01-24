#pragma once

#include <string>
#include <vector>

namespace utils {

// ファイル操作ユーティリティ
class FileUtils {
public:
    // ファイルの内容を読み込む
    static std::string read_file(const std::string& path);

    // ファイルに書き込む
    static void write_file(const std::string& path, const std::string& content);

    // ファイルが存在するか確認
    static bool file_exists(const std::string& path);

    // ディレクトリが存在するか確認
    static bool directory_exists(const std::string& path);

    // ディレクトリを作成（親ディレクトリも作成）
    static void create_directories(const std::string& path);

    // 絶対パスを取得
    static std::string absolute_path(const std::string& path);

    // 相対パスを取得
    static std::string relative_path(const std::string& path, const std::string& base);

    // パスを正規化
    static std::string normalize_path(const std::string& path);

    // ディレクトリ名を取得
    static std::string dirname(const std::string& path);

    // ファイル名を取得
    static std::string basename(const std::string& path);

    // 拡張子を取得
    static std::string extension(const std::string& path);

    // 拡張子を除いたファイル名を取得
    static std::string stem(const std::string& path);

    // パスを結合
    static std::string join_path(const std::string& base, const std::string& path);

    // ディレクトリ内のファイルを列挙
    static std::vector<std::string> list_files(const std::string& directory,
                                               bool recursive = false);

    // パターンに一致するファイルを検索
    static std::vector<std::string> glob(const std::string& pattern);

    // グロブパターンマッチング
    static bool glob_match(const std::string& path, const std::string& pattern);

private:
    FileUtils() = default;
};

}  // namespace utils
