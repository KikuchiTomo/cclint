#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "config/config_loader.hpp"
#include "engine/analysis_engine.hpp"

namespace cclint {
namespace lsp {

/// LSPメッセージ
struct LSPMessage {
    std::string method;
    std::string params;
    int id = -1;  // -1 = notification (no response needed)
};

/// LSPレスポンス
struct LSPResponse {
    int id;
    std::string result;
    std::string error;
};

/// Language Server Protocol サーバー
class LSPServer {
public:
    LSPServer();
    ~LSPServer();

    /// サーバーを起動
    /// @param input_stream 入力ストリーム（通常はstdin）
    /// @param output_stream 出力ストリーム（通常はstdout）
    void run(std::istream& input_stream, std::ostream& output_stream);

    /// サーバーを停止
    void stop();

private:
    bool running_ = false;
    std::ostream* output_stream_ = nullptr;

    // ドキュメント管理
    std::map<std::string, std::string> documents_;  // URI -> content

    // 解析エンジン
    std::unique_ptr<engine::AnalysisEngine> analysis_engine_;
    config::Config config_;

    /// LSPメッセージをパース
    LSPMessage parse_message(const std::string& content);

    /// LSPメッセージを処理
    LSPResponse handle_message(const LSPMessage& message);

    /// レスポンスを送信
    void send_response(const LSPResponse& response);

    /// 通知を送信
    void send_notification(const std::string& method, const std::string& params);

    // LSPメソッドハンドラー
    std::string handle_initialize(const std::string& params);
    std::string handle_text_document_did_open(const std::string& params);
    std::string handle_text_document_did_change(const std::string& params);
    std::string handle_text_document_did_save(const std::string& params);
    std::string handle_text_document_did_close(const std::string& params);

    /// ドキュメントを解析して診断を送信
    void analyze_document(const std::string& uri);
};

}  // namespace lsp
}  // namespace cclint
