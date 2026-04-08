#include "lsp_server.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "config/config_loader.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

namespace cclint {
namespace lsp {

LSPServer::LSPServer() {
    // 設定を読み込む
    config::ConfigLoader loader;
    config_ = loader.load(std::nullopt, ".");

    // 解析エンジンを初期化
    analysis_engine_ = std::make_unique<engine::AnalysisEngine>(config_);
}

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

        if (content_length == 0) {
            continue;
        }

        // Read content
        std::string content(content_length, '\0');
        input_stream.read(&content[0], content_length);

        if (input_stream.good()) {
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
    // "method" is 8 chars; find the colon separator, then the quoted value
    size_t method_pos = content.find("\"method\"");
    if (method_pos != std::string::npos) {
        size_t colon_pos = content.find(":", method_pos + 8);
        if (colon_pos != std::string::npos) {
            size_t value_start = content.find("\"", colon_pos + 1);
            size_t value_end = (value_start != std::string::npos)
                                   ? content.find("\"", value_start + 1)
                                   : std::string::npos;
            if (value_start != std::string::npos && value_end != std::string::npos) {
                message.method =
                    content.substr(value_start + 1, value_end - value_start - 1);
            }
        }
    }

    // id を抽出
    // "id" is 4 chars; find the colon separator, then the numeric value
    size_t id_pos = content.find("\"id\"");
    if (id_pos != std::string::npos) {
        size_t value_start = content.find(":", id_pos + 4);
        if (value_start != std::string::npos) {
            size_t value_end = content.find_first_of(",}", value_start + 1);
            if (value_end != std::string::npos) {
                std::string id_str =
                    content.substr(value_start + 1, value_end - value_start - 1);
                try {
                    message.id = std::stoi(id_str);
                } catch (const std::exception&) {
                    // Invalid id value, leave default
                }
            }
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
    if (!output_stream_)
        return;

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

void LSPServer::send_notification(const std::string& method, const std::string& params) {
    if (!output_stream_)
        return;

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
        // Extract URI: "uri" is 5 chars, find colon then quoted value
        size_t uri_colon = params.find(":", uri_pos + 5);
        size_t uri_start = (uri_colon != std::string::npos)
                               ? params.find("\"", uri_colon + 1)
                               : std::string::npos;
        size_t uri_end = (uri_start != std::string::npos)
                             ? params.find("\"", uri_start + 1)
                             : std::string::npos;
        if (uri_start == std::string::npos || uri_end == std::string::npos) {
            return "";
        }
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        // Extract text: "text" is 6 chars, find colon then quoted value
        size_t text_colon = params.find(":", text_pos + 6);
        size_t text_start = (text_colon != std::string::npos)
                                ? params.find("\"", text_colon + 1)
                                : std::string::npos;
        // Use rfind to find the last quote (end of text value)
        size_t text_end = (text_start != std::string::npos)
                              ? params.rfind("\"")
                              : std::string::npos;
        if (text_start == std::string::npos || text_end == std::string::npos ||
            text_end <= text_start) {
            return "";
        }
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
        // "uri" is 5 chars, find colon then quoted value
        size_t uri_colon = params.find(":", uri_pos + 5);
        size_t uri_start = (uri_colon != std::string::npos)
                               ? params.find("\"", uri_colon + 1)
                               : std::string::npos;
        size_t uri_end = (uri_start != std::string::npos)
                             ? params.find("\"", uri_start + 1)
                             : std::string::npos;
        if (uri_start == std::string::npos || uri_end == std::string::npos) {
            return "";
        }
        std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);

        // Full document update (simplification)
        size_t text_pos = params.find("\"text\"");
        if (text_pos != std::string::npos) {
            size_t text_colon = params.find(":", text_pos + 6);
            size_t text_start = (text_colon != std::string::npos)
                                    ? params.find("\"", text_colon + 1)
                                    : std::string::npos;
            size_t text_end = (text_start != std::string::npos)
                                  ? params.rfind("\"")
                                  : std::string::npos;
            if (text_start != std::string::npos && text_end != std::string::npos &&
                text_end > text_start) {
                std::string text =
                    params.substr(text_start + 1, text_end - text_start - 1);

                documents_[uri] = text;
                analyze_document(uri);
            }
        }
    }

    return "";
}

std::string LSPServer::handle_text_document_did_save(const std::string& params) {
    size_t uri_pos = params.find("\"uri\"");
    if (uri_pos != std::string::npos) {
        size_t uri_colon = params.find(":", uri_pos + 5);
        size_t uri_start = (uri_colon != std::string::npos)
                               ? params.find("\"", uri_colon + 1)
                               : std::string::npos;
        size_t uri_end = (uri_start != std::string::npos)
                             ? params.find("\"", uri_start + 1)
                             : std::string::npos;
        if (uri_start != std::string::npos && uri_end != std::string::npos) {
            std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);
            analyze_document(uri);
        }
    }

    return "";
}

std::string LSPServer::handle_text_document_did_close(const std::string& params) {
    size_t uri_pos = params.find("\"uri\"");
    if (uri_pos != std::string::npos) {
        size_t uri_colon = params.find(":", uri_pos + 5);
        size_t uri_start = (uri_colon != std::string::npos)
                               ? params.find("\"", uri_colon + 1)
                               : std::string::npos;
        size_t uri_end = (uri_start != std::string::npos)
                             ? params.find("\"", uri_start + 1)
                             : std::string::npos;
        if (uri_start != std::string::npos && uri_end != std::string::npos) {
            std::string uri = params.substr(uri_start + 1, uri_end - uri_start - 1);
            documents_.erase(uri);
        }
    }

    return "";
}

void LSPServer::analyze_document(const std::string& uri) {
    auto it = documents_.find(uri);
    if (it == documents_.end()) {
        return;
    }

    // URIをファイルパスに変換（file:// スキームを削除）
    std::string file_path = uri;
    if (file_path.find("file://") == 0) {
        file_path = file_path.substr(7);
    }

    // 一時ファイルに内容を書き込む（メモリ上のコンテンツを解析するため）
    std::string temp_path =
        "/tmp/cclint_lsp_" + std::to_string(std::hash<std::string>{}(uri)) + ".cpp";
    try {
        std::ofstream temp_file(temp_path);
        temp_file << it->second;
        temp_file.close();

        // ファイルを解析
        auto results = analysis_engine_->analyze_files({temp_path});

        // 診断メッセージを収集
        auto all_diagnostics = analysis_engine_->get_all_diagnostics();

        // JSON形式で診断を送信
        std::ostringstream diagnostics_json;
        diagnostics_json << "{\"uri\":\"" << uri << "\",\"diagnostics\":[";

        bool first = true;
        for (const auto& diag : all_diagnostics) {
            if (!first)
                diagnostics_json << ",";
            first = false;

            // Severityを変換
            int severity = 2;  // Warning
            switch (diag.severity) {
                case diagnostic::Severity::Error:
                    severity = 1;
                    break;
                case diagnostic::Severity::Warning:
                    severity = 2;
                    break;
                case diagnostic::Severity::Info:
                    severity = 3;
                    break;
                case diagnostic::Severity::Note:
                    severity = 4;
                    break;
            }

            diagnostics_json << "{";
            diagnostics_json << "\"range\":{";
            diagnostics_json << "\"start\":{\"line\":" << (diag.location.line - 1)
                             << ",\"character\":" << diag.location.column << "},";
            diagnostics_json << "\"end\":{\"line\":" << (diag.location.line - 1)
                             << ",\"character\":" << (diag.location.column + 10) << "}";
            diagnostics_json << "},";
            diagnostics_json << "\"severity\":" << severity << ",";
            diagnostics_json << "\"source\":\"cclint\",";
            diagnostics_json << "\"message\":\"" << diag.message << "\"";
            diagnostics_json << "}";
        }

        diagnostics_json << "]}";

        send_notification("textDocument/publishDiagnostics", diagnostics_json.str());

        // 一時ファイルを削除
        std::filesystem::remove(temp_path);

        utils::Logger::instance().debug("LSP: Analyzed " + uri + " - found " +
                                        std::to_string(all_diagnostics.size()) + " diagnostics");

    } catch (const std::exception& e) {
        utils::Logger::instance().error("LSP: Failed to analyze " + uri + ": " + e.what());
    }
}

}  // namespace lsp
}  // namespace cclint
