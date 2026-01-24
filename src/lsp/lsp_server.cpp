#include "lsp_server.hpp"
#include "utils/logger.hpp"

#include <sstream>
#include <iostream>

namespace cclint {
namespace lsp {

LSPServer::LSPServer() {}

LSPServer::~LSPServer() {
    stop();
}

void LSPServer::run(std::istream& input_stream, std::ostream& output_stream) {
    running_ = true;
    output_stream_ = &output_stream;

    utils::Logger::instance().info("LSP Server starting...");

    while (running_) {
        // Read Content-Length header
        std::string header;
        if (!std::getline(input_stream, header)) {
            break;
        }

        // Parse Content-Length
        size_t content_length = 0;
        if (header.find("Content-Length:") == 0) {
            content_length = std::stoul(header.substr(15));
        }

        // Skip empty line
        std::string empty_line;
        std::getline(input_stream, empty_line);

        // Read content
        std::string content(content_length, '\0');
        input_stream.read(&content[0], content_length);

        if (content_length > 0) {
            // Parse and handle message
            LSPMessage message = parse_message(content);
            LSPResponse response = handle_message(message);

            // Send response (only if id >= 0, meaning it's a request)
            if (response.id >= 0) {
                send_response(response);
            }
        }
    }

    utils::Logger::instance().info("LSP Server stopped");
}

void LSPServer::stop() {
    running_ = false;
}

LSPMessage LSPServer::parse_message(const std::string& content) {
    LSPMessage message;

    // 簡易JSON解析（実際にはJSON parserを使うべき）
    // ここではmethod, params, idを抽出する簡易実装

    // method を抽出
    size_t method_pos = content.find("\"method\"");
    if (method_pos != std::string::npos) {
        size_t value_start = content.find("\"", method_pos + 9);
        size_t value_end = content.find("\"", value_start + 1);
        if (value_start != std::string::npos && value_end != std::string::npos) {
            message.method = content.substr(value_start + 1, value_end - value_start - 1);
        }
    }

    // id を抽出
    size_t id_pos = content.find("\"id\"");
    if (id_pos != std::string::npos) {
        size_t value_start = content.find(":", id_pos + 4);
        size_t value_end = content.find_first_of(",}", value_start + 1);
        if (value_start != std::string::npos && value_end != std::string::npos) {
            std::string id_str = content.substr(value_start + 1, value_end - value_start - 1);
            message.id = std::stoi(id_str);
        }
    }

    // params を抽出（簡易実装）
    message.params = content;

    return message;
}

LSPResponse LSPServer::handle_message(const LSPMessage& message) {
    LSPResponse response;
    response.id = message.id;

    utils::Logger::instance().debug("LSP: Received " + message.method);

    if (message.method == "initialize") {
        response.result = handle_initialize(message.params);
    } else if (message.method == "textDocument/didOpen") {
        handle_text_document_did_open(message.params);
        response.id = -1;  // notification, no response
    } else if (message.method == "textDocument/didChange") {
        handle_text_document_did_change(message.params);
        response.id = -1;  // notification, no response
    } else if (message.method == "textDocument/didSave") {
        handle_text_document_did_save(message.params);
        response.id = -1;  // notification, no response
    } else if (message.method == "textDocument/didClose") {
        handle_text_document_did_close(message.params);
        response.id = -1;  // notification, no response
    } else if (message.method == "shutdown") {
        response.result = "{}";
    } else if (message.method == "exit") {
        stop();
        response.id = -1;  // notification, no response
    } else {
        // Unknown method
        response.error = "{\"code\": -32601, \"message\": \"Method not found\"}";
    }

    return response;
}

void LSPServer::send_response(const LSPResponse& response) {
    if (!output_stream_) return;

    std::ostringstream json;
    json << "{\"jsonrpc\":\"2.0\",\"id\":" << response.id;

    if (!response.error.empty()) {
        json << ",\"error\":" << response.error;
    } else {
        json << ",\"result\":" << response.result;
    }

    json << "}";

    std::string content = json.str();
    *output_stream_ << "Content-Length: " << content.length() << "\r\n\r\n";
    *output_stream_ << content;
    output_stream_->flush();
}

void LSPServer::send_notification(const std::string& method,
                                  const std::string& params) {
    if (!output_stream_) return;

    std::ostringstream json;
    json << "{\"jsonrpc\":\"2.0\",\"method\":\"" << method << "\"";
    if (!params.empty()) {
        json << ",\"params\":" << params;
    }
    json << "}";

    std::string content = json.str();
    *output_stream_ << "Content-Length: " << content.length() << "\r\n\r\n";
    *output_stream_ << content;
    output_stream_->flush();
}

std::string LSPServer::handle_initialize(const std::string& params) {
    (void)params;

    // サーバーの能力を返す
    return R"({
        "capabilities": {
            "textDocumentSync": {
                "openClose": true,
                "change": 2,
                "save": true
            },
            "diagnosticProvider": {
                "interFileDependencies": false,
                "workspaceDiagnostics": false
            }
        },
        "serverInfo": {
            "name": "cclint-lsp",
            "version": "0.1.0"
        }
    })";
}

std::string LSPServer::handle_text_document_did_open(const std::string& params) {
    // URI と content を抽出（簡易実装）
    size_t uri_pos = params.find("\"uri\"");
    size_t text_pos = params.find("\"text\"");

    if (uri_pos != std::string::npos && text_pos != std::string::npos) {
        // Extract URI
        size_t uri_start = params.find("\"", uri_pos + 5);
        size_t uri_end = params.find("\"", uri_start + 1);
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        // Extract text
        size_t text_start = params.find("\"", text_pos + 6);
        size_t text_end = params.rfind("\"");
        std::string text = params.substr(text_start + 1, text_end - text_start - 1);

        documents_[uri] = text;
        analyze_document(uri);
    }

    return "";
}

std::string LSPServer::handle_text_document_did_change(const std::string& params) {
    // URI と contentChanges を抽出（簡易実装）
    size_t uri_pos = params.find("\"uri\"");
    if (uri_pos != std::string::npos) {
        size_t uri_start = params.find("\"", uri_pos + 5);
        size_t uri_end = params.find("\"", uri_start + 1);
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        // Full document update (simplification)
        size_t text_pos = params.find("\"text\"");
        if (text_pos != std::string::npos) {
            size_t text_start = params.find("\"", text_pos + 6);
            size_t text_end = params.rfind("\"");
            std::string text = params.substr(text_start + 1, text_end - text_start - 1);

            documents_[uri] = text;
            analyze_document(uri);
        }
    }

    return "";
}

std::string LSPServer::handle_text_document_did_save(const std::string& params) {
    size_t uri_pos = params.find("\"uri\"");
    if (uri_pos != std::string::npos) {
        size_t uri_start = params.find("\"", uri_pos + 5);
        size_t uri_end = params.find("\"", uri_start + 1);
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        analyze_document(uri);
    }

    return "";
}

std::string LSPServer::handle_text_document_did_close(const std::string& params) {
    size_t uri_pos = params.find("\"uri\"");
    if (uri_pos != std::string::npos) {
        size_t uri_start = params.find("\"", uri_pos + 5);
        size_t uri_end = params.find("\"", uri_start + 1);
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        documents_.erase(uri);
    }

    return "";
}

void LSPServer::analyze_document(const std::string& uri) {
    // ドキュメントを解析して診断を送信
    // 実際には AnalysisEngine を使って解析する

    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        return;
    }

    // 簡易実装: 診断なし（空の配列）を返す
    std::ostringstream diagnostics_json;
    diagnostics_json << "{\"uri\":\"" << uri << "\",\"diagnostics\":[]}";

    send_notification("textDocument/publishDiagnostics", diagnostics_json.str());

    utils::Logger::instance().debug("LSP: Analyzed " + uri);
}

} // namespace lsp
} // namespace cclint
