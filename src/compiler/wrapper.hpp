#pragma once

#include <string>
#include <vector>

namespace compiler {

// コンパイラコマンドの実行結果
struct CompileResult {
    int exit_code = 0;
    std::string stdout_output;
    std::string stderr_output;
    std::vector<std::string> source_files;
    std::vector<std::string> compiler_flags;
};

// コンパイラコマンドをラップして実行
class CompilerWrapper {
public:
    explicit CompilerWrapper(const std::vector<std::string>& command);

    // コンパイラコマンドを実行
    CompileResult execute();

    // ソースファイルのリストを取得（実行せずに解析のみ）
    std::vector<std::string> extract_source_files() const;

    // コンパイラフラグを取得（実行せずに解析のみ）
    std::vector<std::string> extract_compiler_flags() const;

private:
    std::vector<std::string> command_;

    // コマンドからソースファイルを抽出
    std::vector<std::string> parse_source_files() const;

    // コマンドからコンパイラフラグを抽出
    std::vector<std::string> parse_compiler_flags() const;

    // ファイル拡張子がC/C++ソースファイルか判定
    static bool is_source_file(const std::string& filename);

    // ファイル拡張子がC/C++ヘッダファイルか判定
    static bool is_header_file(const std::string& filename);

    // コンパイラフラグか判定
    static bool is_compiler_flag(const std::string& arg);
};

}  // namespace compiler
