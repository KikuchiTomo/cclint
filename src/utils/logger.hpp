#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

namespace utils {

// ログレベル
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

// ロガー
class Logger {
public:
    static Logger& instance();

    // ログレベルを設定
    void set_level(LogLevel level);

    // ログレベルを取得
    LogLevel get_level() const {
        return level_;
    }

    // 出力先を設定
    void set_output(std::ostream* out, std::ostream* err = nullptr);

    // ログファイルを設定
    void set_log_file(const std::string& filepath);

    // ログ出力
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    // フォーマット付きログ出力（簡易版）
    template<typename... Args>
    void debug(const std::string& format, Args... args);

    template<typename... Args>
    void info(const std::string& format, Args... args);

    template<typename... Args>
    void warning(const std::string& format, Args... args);

    template<typename... Args>
    void error(const std::string& format, Args... args);

private:
    Logger();
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& message);

    LogLevel level_ = LogLevel::Info;
    std::ostream* out_ = &std::cout;
    std::ostream* err_ = &std::cerr;
    std::unique_ptr<std::ofstream> log_file_;
};

// グローバル関数
inline void set_log_level(LogLevel level) {
    Logger::instance().set_level(level);
}

inline void log_debug(const std::string& message) {
    Logger::instance().debug(message);
}

inline void log_info(const std::string& message) {
    Logger::instance().info(message);
}

inline void log_warning(const std::string& message) {
    Logger::instance().warning(message);
}

inline void log_error(const std::string& message) {
    Logger::instance().error(message);
}

}  // namespace utils
