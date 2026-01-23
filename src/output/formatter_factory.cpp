#include "formatter_factory.hpp"
#include "json_formatter.hpp"
#include "text_formatter.hpp"
#include "xml_formatter.hpp"

#include <algorithm>
#include <stdexcept>

namespace output {

std::unique_ptr<Formatter> FormatterFactory::create(
    const std::string& format_name) {
    std::string format = format_name;

    // 小文字に変換
    std::transform(format.begin(), format.end(), format.begin(),
                  [](unsigned char c) { return std::tolower(c); });

    if (format == "text" || format == "txt") {
        return std::make_unique<TextFormatter>();
    } else if (format == "json") {
        return std::make_unique<JsonFormatter>();
    } else if (format == "xml") {
        return std::make_unique<XmlFormatter>();
    } else {
        throw std::invalid_argument("Unknown output format: " + format_name);
    }
}

std::vector<std::string> FormatterFactory::get_supported_formats() {
    return {"text", "json", "xml"};
}

bool FormatterFactory::is_valid_format(const std::string& format_name) {
    auto formats = get_supported_formats();
    std::string format = format_name;

    // 小文字に変換
    std::transform(format.begin(), format.end(), format.begin(),
                  [](unsigned char c) { return std::tolower(c); });

    return std::find(formats.begin(), formats.end(), format) != formats.end() ||
           format == "txt";  // "txt" も "text" のエイリアスとして許可
}

}  // namespace output
