#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "configurationWindow.h"
#include <nlohmann/json.hpp>
#include "textUtil.h"

enum EWMHtype
{
    EWMH_WINDOW_TYPE_DESKTOP,
    EWMH_WINDOW_TYPE_DOCK,
    EWMH_WINDOW_TYPE_TOOLBAR,
    EWMH_WINDOW_TYPE_MENU,
    EWMH_WINDOW_TYPE_UTILITY,
    EWMH_WINDOW_TYPE_SPLASH,
    EWMH_WINDOW_TYPE_DIALOG,
    EWMH_WINDOW_TYPE_NORMAL,
    EWMH_WINDOW_TYPE_DROPDOWN_MENU,
    EWMH_WINDOW_TYPE_POPUP_MENU,
    EWMH_WINDOW_TYPE_TOOLTIP,
    EWMH_WINDOW_TYPE_NOTIFICATION,
    EWMH_WINDOW_TYPE_COMBO,
    EWMH_WINDOW_TYPE_DND,
    EWMH_WINDOW_TYPE_UNKNOWN
};

enum EWMHSTATES
{
    EWMH_STATE_MODAL,
    EWMH_STATE_STICKY,
    EWMH_STATE_MAXIMIZED_VERT,
    EWMH_STATE_MAXIMIZED_HORZ,
    EWMH_STATE_SHADED,
    EWMH_STATE_SKIP_TASKBAR,
    EWMH_STATE_SKIP_PAGER,
    EWMH_STATE_HIDDEN,
    EWMH_STATE_FULLSCREEN,
    EWMH_STATE_ABOVE,
    EWMH_STATE_BELOW,
    EWMH_STATE_DEMANDS_ATTENTION,
    EWMH_STATE_FOCUSED,
    EWMH_STATE_NORMAL
};

// Structure pour représenter les hints WM_NORMAL_HINTS
struct WMNormalHints {
    int flags;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
    int base_width;
    int base_height;
    int width_inc;
    int height_inc;
    int min_aspect_num;
    int min_aspect_den;
    int max_aspect_num;
    int max_aspect_den;
    int win_gravity;
};

struct Window
{
    ConfigurationWindow normalConfiguration;
    std::vector<EWMHSTATES> atoms;
    std::vector<WMNormalHints> normalHints;
    bool isOverrideRedirect;
    int transientFor{0};
    EWMHtype type;
    int order{0};
    int pid{0};
};

std::string serializeWindow(const Window &window);
Window deserializeWindow(const std::string &serializedWindow);
std::string serializeWindows(const std::vector<Window> &windows);
std::vector<Window> deserializeWindows(const std::string &serializedWindows);
std::string windowToString(const Window &window);

std::string serializeHint(const WMNormalHints &normalHint);
WMNormalHints deserializeHint(const std::string &serializedNormalHint);
std::string serializeHints(const std::vector<WMNormalHints> &normalHints);
std::vector<WMNormalHints> deserializeWMNormalHints(const std::string &serializedNormalHints);
