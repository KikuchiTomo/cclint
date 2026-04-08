// Intentional violations: restriction rules
// Expected: no_global_using_namespace, no_malloc_in_class

#pragma once

#include <cstdlib>
#include <cstring>

// BAD: global using namespace
using namespace std;

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() { deallocate(); }

    void allocate(size_t size) {
        // BAD: malloc in class method
        buffer_ = static_cast<char*>(malloc(size));
        if (buffer_) {
            memset(buffer_, 0, size);
            size_ = size;
        }
    }

    void deallocate() {
        // BAD: free in class method
        free(buffer_);
        buffer_ = nullptr;
        size_ = 0;
    }

    size_t size() const { return size_; }

private:
    char* buffer_ = nullptr;
    size_t size_ = 0;
};
