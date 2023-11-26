#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
namespace microWM
{
    struct Display
    {
        /* data */;
        int width;
        int height;
        int x;
        int y;
    };
    


    std::string serializeDisplay(const Display *display);
    Display deserializeDisplay(const std::string& serializedDisplay);
    std::string serializeDisplays(const std::vector<Display>& displays);
    std::vector<Display> deserializeDisplays( const std::string& serializedDisplays);
}