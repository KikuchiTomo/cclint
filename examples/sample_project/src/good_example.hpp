// This file should pass all rules cleanly.

#pragma once

#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class HttpClient {
public:
    explicit HttpClient(int timeout) : timeout_(timeout) {}
    virtual ~HttpClient() = default;

    void send_request() { is_connected_ = true; }
    void close_connection() { is_connected_ = false; }
    int timeout() const { return timeout_; }
    bool connected() const { return is_connected_; }

private:
    int timeout_;
    bool is_connected_ = false;
};
