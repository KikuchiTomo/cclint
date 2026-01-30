#include "parser/parser_performance.hpp"

#include <sstream>

namespace cclint {
namespace parser {

void ParserPerformance::start_timer(const std::string& name) {
    auto& timer = timers_[name];
    timer.start = std::chrono::steady_clock::now();
    timer.running = true;
}

void ParserPerformance::stop_timer(const std::string& name) {
    auto it = timers_.find(name);
    if (it != timers_.end() && it->second.running) {
        it->second.end = std::chrono::steady_clock::now();
        it->second.running = false;
    }
}

int64_t ParserPerformance::get_elapsed_ms(const std::string& name) const {
    auto it = timers_.find(name);
    if (it != timers_.end() && !it->second.running) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            it->second.end - it->second.start);
        return duration.count();
    }
    return 0;
}

void ParserPerformance::reset() {
    timers_.clear();
    token_count_ = 0;
    node_count_ = 0;
    memory_usage_ = 0;
}

std::string ParserPerformance::get_stats() const {
    std::ostringstream oss;

    oss << "Parser Performance Statistics:\n";
    oss << "==============================\n";

    // タイマー情報
    if (!timers_.empty()) {
        oss << "\nTimings:\n";
        for (const auto& [name, timer] : timers_) {
            if (!timer.running) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    timer.end - timer.start);
                oss << "  " << name << ": " << duration.count() << " ms\n";
            }
        }
    }

    // トークン・ノード統計
    oss << "\nCounts:\n";
    oss << "  Tokens: " << token_count_ << "\n";
    oss << "  AST Nodes: " << node_count_ << "\n";

    // メモリ使用量
    if (memory_usage_ > 0) {
        oss << "\nMemory:\n";
        if (memory_usage_ < 1024) {
            oss << "  Usage: " << memory_usage_ << " bytes\n";
        } else if (memory_usage_ < 1024 * 1024) {
            oss << "  Usage: " << (memory_usage_ / 1024.0) << " KB\n";
        } else {
            oss << "  Usage: " << (memory_usage_ / (1024.0 * 1024.0)) << " MB\n";
        }
    }

    // スループット
    double tokens_per_sec = get_tokens_per_second();
    double nodes_per_sec = get_nodes_per_second();

    if (tokens_per_sec > 0 || nodes_per_sec > 0) {
        oss << "\nThroughput:\n";
        if (tokens_per_sec > 0) {
            oss << "  Tokens/sec: " << static_cast<int>(tokens_per_sec) << "\n";
        }
        if (nodes_per_sec > 0) {
            oss << "  Nodes/sec: " << static_cast<int>(nodes_per_sec) << "\n";
        }
    }

    return oss.str();
}

double ParserPerformance::get_tokens_per_second() const {
    auto total_ms = get_elapsed_ms("total");
    if (total_ms > 0 && token_count_ > 0) {
        return (token_count_ * 1000.0) / total_ms;
    }
    return 0.0;
}

double ParserPerformance::get_nodes_per_second() const {
    auto total_ms = get_elapsed_ms("total");
    if (total_ms > 0 && node_count_ > 0) {
        return (node_count_ * 1000.0) / total_ms;
    }
    return 0.0;
}

}  // namespace parser
}  // namespace cclint
