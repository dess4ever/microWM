#include "window.h"

using json = nlohmann::json;

// Convertit un EWMHtype enum en string
std::string ewmhTypeToString(EWMHtype type) {
    switch (type) {
        case EWMH_WINDOW_TYPE_DESKTOP: return "DESKTOP";
        case EWMH_WINDOW_TYPE_DOCK: return "DOCK";
        case EWMH_WINDOW_TYPE_TOOLBAR: return "TOOLBAR";
        case EWMH_WINDOW_TYPE_MENU: return "MENU";
        case EWMH_WINDOW_TYPE_UTILITY: return "UTILITY";
        case EWMH_WINDOW_TYPE_SPLASH: return "SPLASH";
        case EWMH_WINDOW_TYPE_DIALOG: return "DIALOG";
        case EWMH_WINDOW_TYPE_NORMAL: return "NORMAL";
        case EWMH_WINDOW_TYPE_DROPDOWN_MENU: return "DROPDOWN_MENU";
        case EWMH_WINDOW_TYPE_POPUP_MENU: return "POPUP_MENU";
        case EWMH_WINDOW_TYPE_TOOLTIP: return "TOOLTIP";
        case EWMH_WINDOW_TYPE_NOTIFICATION: return "NOTIFICATION";
        case EWMH_WINDOW_TYPE_COMBO: return "COMBO";
        case EWMH_WINDOW_TYPE_DND: return "DND";
        default: return "UNKNOWN";
    }
}

// Convertit un string en EWMHtype enum
EWMHtype stringToEwmhType(const std::string& str) {
    if (str == "DESKTOP") return EWMH_WINDOW_TYPE_DESKTOP;
    if (str == "DOCK") return EWMH_WINDOW_TYPE_DOCK;
    if (str == "TOOLBAR") return EWMH_WINDOW_TYPE_TOOLBAR;
    if (str == "MENU") return EWMH_WINDOW_TYPE_MENU;
    if (str == "UTILITY") return EWMH_WINDOW_TYPE_UTILITY;
    if (str == "SPLASH") return EWMH_WINDOW_TYPE_SPLASH;
    if (str == "DIALOG") return EWMH_WINDOW_TYPE_DIALOG;
    if (str == "NORMAL") return EWMH_WINDOW_TYPE_NORMAL;
    if (str == "DROPDOWN_MENU") return EWMH_WINDOW_TYPE_DROPDOWN_MENU;
    if (str == "POPUP_MENU") return EWMH_WINDOW_TYPE_POPUP_MENU;
    if (str == "TOOLTIP") return EWMH_WINDOW_TYPE_TOOLTIP;
    if (str == "NOTIFICATION") return EWMH_WINDOW_TYPE_NOTIFICATION;
    if (str == "COMBO") return EWMH_WINDOW_TYPE_COMBO;
    if (str == "DND") return EWMH_WINDOW_TYPE_DND;
    // Assumez que tout type non reconnu est UNKNOWN
    return EWMH_WINDOW_TYPE_UNKNOWN;
}

// Sérialise un objet WMNormalHints en string JSON
std::string serializeHint(const WMNormalHints &normalHint) {
    json j;
    j["flags"] = normalHint.flags;
    j["min_width"] = normalHint.min_width;
    j["min_height"] = normalHint.min_height;
    j["max_width"] = normalHint.max_width;
    j["max_height"] = normalHint.max_height;
    j["base_width"] = normalHint.base_width;
    j["base_height"] = normalHint.base_height;
    j["width_inc"] = normalHint.width_inc;
    j["height_inc"] = normalHint.height_inc;
    j["min_aspect_num"] = normalHint.min_aspect_num;
    j["min_aspect_den"] = normalHint.min_aspect_den;
    j["max_aspect_num"] = normalHint.max_aspect_num;
    j["max_aspect_den"] = normalHint.max_aspect_den;
    j["win_gravity"] = normalHint.win_gravity;
    return j.dump();
}

// Désérialise un objet JSON en WMNormalHints
WMNormalHints deserializeHint(const json &j) {
    WMNormalHints normalHint;
    normalHint.flags = j["flags"];
    normalHint.min_width = j["min_width"];
    normalHint.min_height = j["min_height"];
    normalHint.max_width = j["max_width"];
    normalHint.max_height = j["max_height"];
    normalHint.base_width = j["base_width"];
    normalHint.base_height = j["base_height"];
    normalHint.width_inc = j["width_inc"];
    normalHint.height_inc = j["height_inc"];
    normalHint.min_aspect_num = j["min_aspect_num"];
    normalHint.min_aspect_den = j["min_aspect_den"];
    normalHint.max_aspect_num = j["max_aspect_num"];
    normalHint.max_aspect_den = j["max_aspect_den"];
    normalHint.win_gravity = j["win_gravity"];
    return normalHint;
}

// Sérialise un vecteur de WMNormalHints en JSON string
std::string serializeHints(const std::vector<WMNormalHints> &normalHints) {
    json j = json::array();
    for (const WMNormalHints &hint : normalHints) {
        j.push_back(json::parse(serializeHint(hint)));
    }
    return j.dump();
}

// Désérialise un vecteur de WMNormalHints à partir d'une JSON string
std::vector<WMNormalHints> deserializeWMNormalHints(const std::string &serializedNormalHints) {
    std::vector<WMNormalHints> normalHints;
    json j = json::parse(serializedNormalHints);
    for (const json &serializedHint : j) {
        normalHints.push_back(deserializeHint(serializedHint));
    }
    return normalHints;
}

// Fonction utilitaire pour sérialiser un vecteur d'EWMHState
json serializeEWMHStates(const std::vector<EWMHSTATES>& atoms) {
    json jStates = json::array();
    for (EWMHSTATES state : atoms) {
        jStates.push_back(static_cast<int>(state)); // Convertit l'enum en int pour le JSON
    }
    return jStates;
}

// Fonction utilitaire pour désérialiser un vecteur d'EWMHState
std::vector<EWMHSTATES> deserializeEWMHSTATES(const json& jAtoms) {
    std::vector<EWMHSTATES> states;
    for (const auto& jState : jAtoms) {
        states.push_back(static_cast<EWMHSTATES>(jState.get<int>()));
    }
    return states;
}

// Sérialise un objet Window en string JSON
std::string serializeWindow(const Window &window) {
    json j;
    j["normalConfiguration"] = json::parse(serializeConfigurationWindow(window.normalConfiguration));
    j["states"] = serializeEWMHStates(window.atoms);
    j["normalHints"] =  json::parse(serializeHints(window.normalHints));  // À remplacer par la sérialisation réelle des normalHints
    j["isOverrideRedirect"] = window.isOverrideRedirect;
    j["transientFor"] = window.transientFor;
    j["type"] = ewmhTypeToString(window.type);
    j["pid"] = window.pid;
    return j.dump();
}

// Désérialise une string JSON en objet Window
Window deserializeWindow(const std::string &serializedWindow) {
    json j = json::parse(serializedWindow);
    Window window;
    window.normalConfiguration = deserializeConfigurationWindow(j["normalConfiguration"].dump());
    window.atoms = deserializeEWMHSTATES(j["states"]);
    window.isOverrideRedirect = j["isOverrideRedirect"].get<bool>();
    window.transientFor = j["transientFor"].get<int>();
    window.type = stringToEwmhType(j["type"].get<std::string>());
    window.pid = j["pid"].get<int>();
    window.normalHints = deserializeWMNormalHints(j["normalHints"].dump());
    return window;
}

// Sérialise un vecteur d'objets Window en string JSON
std::string serializeWindows(const std::vector<Window> &windows) {
    json j = json::array();
    for (const Window& window : windows) {
        j.push_back(json::parse(serializeWindow(window)));
    }
    return j.dump();
}

// Désérialise une string JSON en vecteur d'objets Window
std::vector<Window> deserializeWindows(const std::string &serializedWindows) {
    std::vector<Window> windows;
    json j = json::parse(serializedWindows);
    for (const auto& serializedWindow : j) {
        windows.push_back(deserializeWindow(serializedWindow.dump()));
    }
    return windows;
}

// Convertit un objet Window en string descriptive (pour le debug, par exemple)
std::string windowToString(const Window &window) {
    std::stringstream ss;
    ss << "Window Information: \n";
    ss << "  Window ID: " << window.normalConfiguration.windowId << "\n";
    ss << "  Type: " << ewmhTypeToString(window.type) << "\n";
    ss << "  PID: " << window.pid << "\n";
    ss << "  Order: " << window.order << "\n";
    ss << "  Is Override Redirect: " << (window.isOverrideRedirect ? "Yes" : "No") << "\n";
    ss << "  Transient For: " << window.transientFor << "\n";
    ss << "  States: ";
    for (EWMHSTATES state : window.atoms) {
        ss << state << " "; 
    }
    ss << "\n";
    ss << "  Normal Hints: \n";
    for (const WMNormalHints &hint : window.normalHints) {
        ss << "    Flags: " << hint.flags << "\n";
        ss << "    Min Width: " << hint.min_width << "\n";
        ss << "    Min Height: " << hint.min_height << "\n";
        ss << "    Max Width: " << hint.max_width << "\n";
        ss << "    Max Height: " << hint.max_height << "\n";
        ss << "    Base Width: " << hint.base_width << "\n";
        ss << "    Base Height: " << hint.base_height << "\n";
        ss << "    Width Increment: " << hint.width_inc << "\n";
        ss << "    Height Increment: " << hint.height_inc << "\n";
        ss << "    Min Aspect Num: " << hint.min_aspect_num << "\n";
        ss << "    Min Aspect Den: " << hint.min_aspect_den << "\n";
        ss << "    Max Aspect Num: " << hint.max_aspect_num << "\n";
        ss << "    Max Aspect Den: " << hint.max_aspect_den << "\n";
        ss << "    Win Gravity: " << hint.win_gravity << "\n\n";
    }
    return ss.str();
}
