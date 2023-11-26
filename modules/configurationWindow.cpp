#include "configurationWindow.h"

using json = nlohmann::json;

std::string unescape(const std::string& str) {
    std::string unescapedStr;
    bool isEscaping = false;
    for (char c : str) {
        if (isEscaping) {
            unescapedStr += c;
            isEscaping = false;
        } else if (c == '\\') {
            isEscaping = true;
        } else {
            unescapedStr += c;
        }
    }
    return unescapedStr;
}

std::string serializeConfigurationWindow(const ConfigurationWindow &configuration)
{
    json j;
    j["x"] = configuration.x;
    j["y"] = configuration.y;
    j["width"] = configuration.width;
    j["height"] = configuration.height;
    j["borderWidth"] = configuration.borderWidth;
    j["border"] = configuration.border;
    j["sibling"] = configuration.sibling;
    j["stackMode"] = configuration.stackMode;
    j["displayScreen"] = configuration.displayScreen;
    j["windowClass"] = sanitize_utf8(configuration.windowClass);
    j["windowName"] = sanitize_utf8 (configuration.windowName);
    j["windowId"] = configuration.windowId;
    return j.dump();
}

ConfigurationWindow deserializeConfigurationWindow(const std::string &configuration)
{
    json j = json::parse(configuration);
    ConfigurationWindow config;
    config.x = j["x"];
    config.y = j["y"];
    config.width = j["width"];
    config.height = j["height"];
    config.borderWidth = j["borderWidth"];
    config.border = j["border"];
    config.sibling = j["sibling"];
    config.stackMode = j["stackMode"];
    config.displayScreen = j["displayScreen"];
    config.windowClass = j["windowClass"];
    config.windowName = j["windowName"];
    config.windowId = j["windowId"];
    return config;
}

std::string serializeConfigurationWindows(const std::vector<ConfigurationWindow> &configurations)
{
    json j = json::array();
    for (const auto &config : configurations)
    {
        j.push_back(json::parse(serializeConfigurationWindow(config)));
    }
    return j.dump();
}

std::vector<ConfigurationWindow> deserializeConfigurationWindows(const std::string &configurations)
{
    std::vector<ConfigurationWindow> configs;
    json j = json::parse(configurations);
    for (const auto &config : j)
    {
        configs.push_back(deserializeConfigurationWindow(config.dump()));
    }
    return configs;
}

std::string configurationWindowsToString(const ConfigurationWindow &configuration)
{
    return serializeConfigurationWindow(configuration);
}