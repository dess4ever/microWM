#pragma once
#include <vector>
#include "display.h"
#include <iostream>
#include <xcb/xcb.h>
#include <xcb/randr.h>
using Display=microWM::Display;

class DisplayInfo {
    public:
        DisplayInfo(xcb_connection_t *connection);
        ~DisplayInfo();
        void print();
        std::vector<Display> getScreens();
    private:
        xcb_connection_t *connection;
        xcb_randr_get_screen_resources_current_reply_t *screen;
        std::vector<Display> screens;
};