#include "detector.hpp"

#include <array>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <regex>

namespace compiler {

namespace fs = std::filesystem;

CompilerInfo CompilerDetector::detect(const std::vector<std::string>& command) {
    if (command.empty()) {
        return CompilerInfo{};
    }

    std::string compiler_path = command[0];
    fs::path path(compiler_path);
    std::string compiler_name = path.filename().string();

    CompilerInfo info;
    info.name = compiler_name;
    info.path = compiler_path;
    info.type = detect_type(compiler_name);

    // バージョン情報を取得
    try {
        info.version = get_version(compiler_path);

        // バージョン出力から詳細なコンパイラ情報を取得
        auto detailed_info = parse_version_output(info.version, compiler_path);
        if (detailed_info.type != CompilerType::Unknown) {
            info.type = detailed_info.type;
        }
    } catch (...) {
        // バージョン取得に失敗してもエラーにしない
        info.version = "unknown";
    }

    return info;
}

CompilerType CompilerDetector::detect_type(const std::string& compiler_name) {
    // コンパイラ名からタイプを推測
    if (compiler_name.find("clang++") != std::string::npos ||
        compiler_name.find("clang") != std::string::npos) {
        return CompilerType::Clang;
    } else if (compiler_name.find("g++") != std::string::npos ||
               compiler_name.find("gcc") != std::string::npos) {
        return CompilerType::GCC;
    } else if (compiler_name.find("cl.exe") != std::string::npos || compiler_name == "cl") {
        return CompilerType::MSVC;
    } else if (compiler_name.find("c++") != std::string::npos ||
               compiler_name.find("cc") != std::string::npos) {
        // c++ や cc はシステムデフォルトのコンパイラへのシンボリックリンク
        // バージョン情報から判定する必要がある
        return CompilerType::Unknown;
    }

    return CompilerType::Unknown;
}

std::string CompilerDetector::get_version(const std::string& compiler_path) {
    // コンパイラのバージョンを取得
    std::string cmd = compiler_path + " --version 2>&1";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        return "unknown";
    }

    std::string result;
    std::array<char, 256> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

CompilerInfo CompilerDetector::parse_version_output(const std::string& output,
                                                    const std::string& compiler_path) {
    CompilerInfo info;
    info.path = compiler_path;

    // Apple clang の検出
    if (output.find("Apple clang") != std::string::npos ||
        output.find("Apple LLVM") != std::string::npos) {
        info.type = CompilerType::AppleClang;
        info.name = "Apple Clang";

        // バージョン番号を抽出
        std::regex version_regex(R"(version\s+(\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(output, match, version_regex)) {
            info.version = match[1].str();
        }
    }
    // Clang の検出
    else if (output.find("clang version") != std::string::npos) {
        info.type = CompilerType::Clang;
        info.name = "Clang";

        std::regex version_regex(R"(clang version\s+(\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(output, match, version_regex)) {
            info.version = match[1].str();
        }
    }
    // GCC の検出
    else if (output.find("gcc") != std::string::npos || output.find("GCC") != std::string::npos ||
             output.find("g++") != std::string::npos) {
        info.type = CompilerType::GCC;
        info.name = "GCC";

        std::regex version_regex(R"((\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(output, match, version_regex)) {
            info.version = match[1].str();
        }
    }
    // MSVC の検出
    else if (output.find("Microsoft") != std::string::npos) {
        info.type = CompilerType::MSVC;
        info.name = "MSVC";

        std::regex version_regex(R"(Version\s+(\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(output, match, version_regex)) {
            info.version = match[1].str();
        }
    }

    return info;
}

}  // namespace compiler
