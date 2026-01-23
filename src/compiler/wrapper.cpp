#include "wrapper.hpp"

#include <array>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace compiler {

namespace fs = std::filesystem;

CompilerWrapper::CompilerWrapper(const std::vector<std::string>& command)
    : command_(command) {
    if (command_.empty()) {
        throw std::invalid_argument("Compiler command cannot be empty");
    }
}

CompileResult CompilerWrapper::execute() {
    CompileResult result;

    // コマンドライン文字列を構築
    std::ostringstream cmd_stream;
    for (size_t i = 0; i < command_.size(); ++i) {
        if (i > 0) {
            cmd_stream << " ";
        }
        // 引数にスペースが含まれる場合はクォート
        if (command_[i].find(' ') != std::string::npos) {
            cmd_stream << "\"" << command_[i] << "\"";
        } else {
            cmd_stream << command_[i];
        }
    }

    // stdout と stderr を両方キャプチャ
    std::string cmd = cmd_stream.str();
    std::string cmd_with_redirect = cmd + " 2>&1";

    // popenでコマンドを実行
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(cmd_with_redirect.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("Failed to execute compiler command: " + cmd);
    }

    // 出力を読み取り
    std::array<char, 256> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result.stdout_output += buffer.data();
    }

    // 終了コードを取得
    result.exit_code = pclose(pipe.release()) / 256;

    // ソースファイルとフラグを抽出
    result.source_files = parse_source_files();
    result.compiler_flags = parse_compiler_flags();

    return result;
}

std::vector<std::string> CompilerWrapper::extract_source_files() const {
    return parse_source_files();
}

std::vector<std::string> CompilerWrapper::extract_compiler_flags() const {
    return parse_compiler_flags();
}

std::vector<std::string> CompilerWrapper::parse_source_files() const {
    std::vector<std::string> source_files;

    for (size_t i = 1; i < command_.size(); ++i) {  // コンパイラ名をスキップ
        const auto& arg = command_[i];

        // フラグはスキップ
        if (arg.empty() || arg[0] == '-') {
            // -I や -D などのフラグで値を取る場合、次の引数もスキップ
            if (arg == "-I" || arg == "-D" || arg == "-o" ||
                arg == "-std" || arg == "-isystem") {
                ++i;  // 次の引数もスキップ
            }
            continue;
        }

        // ソースファイルまたはヘッダファイルか判定
        if (is_source_file(arg) || is_header_file(arg)) {
            source_files.push_back(arg);
        }
    }

    return source_files;
}

std::vector<std::string> CompilerWrapper::parse_compiler_flags() const {
    std::vector<std::string> flags;

    for (size_t i = 1; i < command_.size(); ++i) {  // コンパイラ名をスキップ
        const auto& arg = command_[i];

        // ソースファイルはスキップ
        if (is_source_file(arg) || is_header_file(arg)) {
            continue;
        }

        // フラグか判定
        if (is_compiler_flag(arg)) {
            flags.push_back(arg);

            // 値を取るフラグの場合、次の引数も追加
            if (arg == "-I" || arg == "-D" || arg == "-o" ||
                arg == "-std" || arg == "-isystem" || arg == "-include") {
                if (i + 1 < command_.size()) {
                    flags.push_back(command_[i + 1]);
                    ++i;
                }
            }
        }
    }

    return flags;
}

bool CompilerWrapper::is_source_file(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }

    fs::path path(filename);
    std::string ext = path.extension().string();

    return ext == ".cpp" || ext == ".cc" || ext == ".cxx" ||
           ext == ".c" || ext == ".C";
}

bool CompilerWrapper::is_header_file(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }

    fs::path path(filename);
    std::string ext = path.extension().string();

    return ext == ".h" || ext == ".hpp" || ext == ".hh" ||
           ext == ".hxx" || ext == ".H";
}

bool CompilerWrapper::is_compiler_flag(const std::string& arg) {
    if (arg.empty()) {
        return false;
    }

    // "-" で始まるものはフラグ
    return arg[0] == '-';
}

}  // namespace compiler
