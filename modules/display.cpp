#include "display.h"

using json = nlohmann::json;
using Display=microWM::Display;

std::string microWM::serializeDisplay(const Display *display)
{
    json j;
    j["width"] = display->width;
    j["height"] = display->height;
    j["x"] = display->x;
    j["y"] = display->y;
    return j.dump();
}

Display microWM::deserializeDisplay(const std::string &serializedDisplay)
{
    json j = json::parse(serializedDisplay);
    Display display;
    display.width = j["width"];
    display.height = j["height"];
    display.x = j["x"];
    display.y = j["y"];
    return display;
}

std::string microWM::serializeDisplays(const std::vector<Display> &displays)
{
    json j = json::array();
    for (const auto &display : displays)
    {
        j.push_back(json::parse(microWM::serializeDisplay(&display)));
    }
    return j.dump();
}

std::vector<Display> microWM::deserializeDisplays(const std::string &serializedDisplays)
{
    std::vector<Display> displays;
    json j = json::parse(serializedDisplays);
    for (const auto &display : j)
    {
        displays.push_back(deserializeDisplay(display.dump()));
    }
    return displays;
}