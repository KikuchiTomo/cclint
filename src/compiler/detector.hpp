#pragma once

#include <string>
#include <vector>

namespace compiler {

// サポートされているコンパイラ
enum class CompilerType { GCC, Clang, MSVC, AppleClang, Unknown };

// コンパイラ情報
struct CompilerInfo {
    CompilerType type = CompilerType::Unknown;
    std::string name;
    std::string version;
    std::string path;
};

// コンパイラを検出
class CompilerDetector {
public:
    CompilerDetector() = default;

    // コマンドからコンパイラを検出
    CompilerInfo detect(const std::vector<std::string>& command);

    // コンパイラ名からコンパイラタイプを推測
    static CompilerType detect_type(const std::string& compiler_name);

    // コンパイラのバージョンを取得
    static std::string get_version(const std::string& compiler_path);

private:
    // コンパイラのバージョン出力をパース
    static CompilerInfo parse_version_output(const std::string& output,
                                             const std::string& compiler_path);
};

}  // namespace compiler
