#pragma once
#include "xcbManager.h"
#include "EWMHManager.h"
#include <iostream>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <xcb/xcb_keysyms.h>
#include <memory>
#include <algorithm>
#include "logManager.h"
#include <map>
#include <functional>

class XCBEventsManager
{
    public:
        XCBEventsManager(LogManager &logManager);
        ~XCBEventsManager();
        xcb_generic_event_t* getEvent();
        xcb_generic_event_t *the_events;
        int getFileDescriptor();
        void handleEvent();
        XcbManager *getXcbManager();
    private:
        LogManager *logManager;
        XcbManager *xcbManager;
        void destroyWindowNotify();
        void unmapWindowNotify();
        void validateMapRequest();
        void configurationRequest();
        void grabNumericKeyboard();
        void ungrabNumericKeyboard(); 
        void expose();
        // Check du nom de l'atome
        auto get_atom_name(xcb_get_atom_name_reply_t* (*reply_func)(xcb_connection_t*, xcb_get_atom_name_cookie_t, xcb_generic_error_t**), xcb_connection_t* conn, xcb_atom_t atom);
        // fonction pour l'optimisation du code (messages)
        template <typename T> auto check_and_apply(const std::string& full_atom_name, const std::string& query_atom_name, T action);
        bool numlock_active{false};
};
