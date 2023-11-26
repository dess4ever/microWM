#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "textUtil.h"

struct ConfigurationWindow
{
    int x{0};
    int y{0};
    int relativeX{0};
    int relativeY{0};
    int width{0};
    int height{0};
    int borderWidth{0};
    int border{0};
    int sibling{-1};
    int stackMode{0};
    int displayScreen{0};
    std::string windowClass{"none"};
    std::string windowName{"none"};
    int windowId{0};
    uint8_t opacity{255};
};

std::string serializeConfigurationWindow(const ConfigurationWindow &configuration);
ConfigurationWindow deserializeConfigurationWindow(const std::string &configuration);
std::string serializeConfigurationWindows(const std::vector<ConfigurationWindow> &configurations);
std::vector<ConfigurationWindow> deserializeConfigurationWindows(const std::string &configurations);
std::string configurationWindowsToString(const ConfigurationWindow& configuration);
