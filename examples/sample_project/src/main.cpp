#include "bad_naming.hpp"
#include "bad_safety.hpp"
#include "bad_restrictions.hpp"
#include "bad_structure.hpp"
#include "good_example.hpp"

#include <cstdio>

int main() {
    // bad_naming
    user_manager um;
    um.add_user("Alice");
    std::printf("users: %zu\n", um.user_count());

    // bad_safety
    Derived d;
    std::printf("name: %s\n", d.name().c_str());
    Connection conn(8080);
    std::printf("port: %d\n", conn.port());

    // bad_restrictions
    ResourceManager rm;
    rm.allocate(256);
    std::printf("allocated: %zu bytes\n", rm.size());

    // bad_structure
    Logger logger;
    Formatter fmt;
    Writer writer;
    logger.log("hello");
    fmt.format("hello");
    writer.write("hello");

    // good_example
    HttpClient client(30);
    client.send_request();
    std::printf("timeout: %d\n", client.timeout());

    return 0;
}
