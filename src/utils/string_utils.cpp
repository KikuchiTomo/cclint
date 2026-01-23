#include "string_utils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace utils {

std::vector<std::string> StringUtils::split(const std::string& str,
                                           char delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        result.push_back(token);
    }

    return result;
}

std::vector<std::string> StringUtils::split(const std::string& str,
                                           const std::string& delimiters) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find_first_of(delimiters);

    while (end != std::string::npos) {
        if (end != start) {
            result.push_back(str.substr(start, end - start));
        }
        start = end + 1;
        end = str.find_first_of(delimiters, start);
    }

    if (start < str.size()) {
        result.push_back(str.substr(start));
    }

    return result;
}

std::string StringUtils::join(const std::vector<std::string>& strings,
                             const std::string& separator) {
    if (strings.empty()) {
        return "";
    }

    std::ostringstream oss;
    oss << strings[0];

    for (size_t i = 1; i < strings.size(); ++i) {
        oss << separator << strings[i];
    }

    return oss.str();
}

std::string StringUtils::trim(const std::string& str) {
    return trim_right(trim_left(str));
}

std::string StringUtils::trim_left(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && is_space(str[start])) {
        ++start;
    }
    return str.substr(start);
}

std::string StringUtils::trim_right(const std::string& str) {
    size_t end = str.size();
    while (end > 0 && is_space(str[end - 1])) {
        --end;
    }
    return str.substr(0, end);
}

std::string StringUtils::to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool StringUtils::starts_with(const std::string& str,
                              const std::string& prefix) {
    if (str.size() < prefix.size()) {
        return false;
    }
    return str.substr(0, prefix.size()) == prefix;
}

bool StringUtils::ends_with(const std::string& str,
                            const std::string& suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return str.substr(str.size() - suffix.size()) == suffix;
}

std::string StringUtils::replace(const std::string& str,
                                const std::string& from,
                                const std::string& to) {
    size_t pos = str.find(from);
    if (pos == std::string::npos) {
        return str;
    }

    std::string result = str;
    result.replace(pos, from.size(), to);
    return result;
}

std::string StringUtils::replace_all(const std::string& str,
                                    const std::string& from,
                                    const std::string& to) {
    if (from.empty()) {
        return str;
    }

    std::string result = str;
    size_t pos = 0;

    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.size(), to);
        pos += to.size();
    }

    return result;
}

bool StringUtils::contains(const std::string& str,
                          const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

bool StringUtils::is_space(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

}  // namespace utils
