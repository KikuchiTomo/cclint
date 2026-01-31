#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

namespace utils {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : level_(LogLevel::Info), out_(&std::cout), err_(&std::cerr) {}

void Logger::set_level(LogLevel level) {
    level_ = level;
}

void Logger::set_output(std::ostream* out, std::ostream* err) {
    if (out) {
        out_ = out;
    }
    if (err) {
        err_ = err;
    }
}

void Logger::set_log_file(const std::string& filepath) {
    log_file_ = std::make_unique<std::ofstream>(filepath, std::ios::app);
    if (log_file_->is_open()) {
        out_ = log_file_.get();
        err_ = log_file_.get();
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    // ログレベルチェック
    if (level < level_) {
        return;
    }

    // 出力先を選択
    std::ostream* stream = (level >= LogLevel::Warning) ? err_ : out_;

    // タイムスタンプ
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif

    // ログレベル文字列
    const char* level_str = "";
    switch (level) {
        case LogLevel::Debug:
            level_str = "DEBUG";
            break;
        case LogLevel::Info:
            level_str = "INFO ";
            break;
        case LogLevel::Warning:
            level_str = "WARN ";
            break;
        case LogLevel::Error:
            level_str = "ERROR";
            break;
    }

    // ログ出力
    *stream << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0')
            << std::setw(3) << ms.count() << "] " << "[" << level_str << "] " << message
            << std::endl;
}

}  // namespace utils
