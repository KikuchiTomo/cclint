// Intentional violations: structure rules
// Expected: one_class_per_file

#pragma once

#include <cstdio>

// BAD: multiple classes in one file
class Logger {
public:
    void log(const char* msg) { std::printf("[LOG] %s\n", msg); }

private:
    int level_ = 0;
};

class Formatter {
public:
    void format(const char* msg) { std::printf("[FMT] %s\n", msg); }

private:
    bool colorize_ = false;
};

class Writer {
public:
    void write(const char* msg) { std::printf("[OUT] %s\n", msg); }

private:
    int fd_ = 1;
};
