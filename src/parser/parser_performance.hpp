#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace cclint {
namespace parser {

/// パーサーパフォーマンス計測
class ParserPerformance {
public:
    /// タイマー開始
    void start_timer(const std::string& name);

    /// タイマー終了
    void stop_timer(const std::string& name);

    /// 経過時間を取得（ミリ秒）
    int64_t get_elapsed_ms(const std::string& name) const;

    /// 統計情報をリセット
    void reset();

    /// 統計情報を文字列として取得
    std::string get_stats() const;

    /// トークン数を記録
    void record_tokens(size_t count) { token_count_ += count; }

    /// AST ノード数を記録
    void record_nodes(size_t count) { node_count_ += count; }

    /// メモリ使用量を記録（バイト）
    void record_memory(size_t bytes) { memory_usage_ += bytes; }

    /// パース速度を計算（トークン/秒）
    double get_tokens_per_second() const;

    /// AST 構築速度を計算（ノード/秒）
    double get_nodes_per_second() const;

private:
    struct Timer {
        std::chrono::steady_clock::time_point start;
        std::chrono::steady_clock::time_point end;
        bool running = false;
    };

    std::unordered_map<std::string, Timer> timers_;
    size_t token_count_ = 0;
    size_t node_count_ = 0;
    size_t memory_usage_ = 0;
};

/// パフォーマンス最適化ヒント
class ParserOptimization {
public:
    /// トークンプールを使用してメモリ割り当てを最適化
    static constexpr size_t TOKEN_POOL_SIZE = 10000;

    /// AST ノードプールを使用してメモリ割り当てを最適化
    static constexpr size_t NODE_POOL_SIZE = 5000;

    /// 先読みトークン数（パフォーマンス vs メモリのトレードオフ）
    static constexpr size_t LOOKAHEAD_COUNT = 4;

    /// キャッシュサイズ（型解決結果のキャッシュ）
    static constexpr size_t TYPE_CACHE_SIZE = 1000;

    /// 並列パース可能な最小ファイルサイズ（バイト）
    static constexpr size_t MIN_PARALLEL_SIZE = 100000;  // 100KB
};

}  // namespace parser
}  // namespace cclint
