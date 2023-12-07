#include "EWMHManager.h"

EWMHManager::EWMHManager(xcb_connection_t *connection)
{
    this->connection = connection;
    EWMHCookie = xcb_ewmh_init_atoms(connection, &EWMH);
    if (!xcb_ewmh_init_atoms_replies(&EWMH, EWMHCookie, NULL))
    {
        std::cerr << "Erreur: Impossible d'initialiser EWMH\n";
        exit(-1);
    }
    std::vector<xcb_atom_t> allowedAtoms;
    allowedAtoms.push_back(EWMH._NET_WM_ACTION_FULLSCREEN);
    allowedAtoms.push_back(EWMH._NET_WM_ACTION_BELOW);
    allowedAtoms.push_back(EWMH._NET_WM_ACTION_ABOVE);
    addAllowedActionsAtomToRootWindow(allowedAtoms);
    xcb_window_t rootWindow = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
    // Je définis le nom du window manager à MicroWM
    xcb_ewmh_set_wm_name(&EWMH, rootWindow, 9, "MicroWM");
    // Je définis les écrans virtuels à 1
    xcb_ewmh_set_number_of_desktops(&EWMH, 0, 1);
    // Je définis l'écran courant à 0
    xcb_ewmh_set_current_desktop(&EWMH, 0, 0);
    // Je définis _net_supporting_wm_check à la fenêtre racine
    xcb_ewmh_set_supporting_wm_check(&EWMH, 0, rootWindow);
    // Je définis _net_desktop_viewport à la fenêtre racine avec comme coordonnées 0,0
    xcb_ewmh_coordinates_t viewport = {0, 0};
    xcb_ewmh_set_desktop_viewport(&EWMH, 0, 1, &viewport);

}
    

EWMHManager::~EWMHManager()
{
    xcb_ewmh_connection_wipe(&EWMH);
}


EWMHtype EWMHManager::getWindowType(xcb_window_t window)
{
    xcb_ewmh_get_atoms_reply_t win_type;
    EWMHtype windowType;
    if (xcb_ewmh_get_wm_window_type_reply(&EWMH, xcb_ewmh_get_wm_window_type(&EWMH, window), &win_type, NULL) == 1)
    {
        if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_DESKTOP)
        {
            windowType = EWMH_WINDOW_TYPE_DESKTOP;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_DOCK)
        {
            windowType = EWMH_WINDOW_TYPE_DOCK;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_TOOLBAR)
        {
            windowType = EWMH_WINDOW_TYPE_TOOLBAR;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_MENU)
        {
            windowType = EWMH_WINDOW_TYPE_MENU;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_UTILITY)
        {
            windowType = EWMH_WINDOW_TYPE_UTILITY;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_SPLASH)
        {
            windowType = EWMH_WINDOW_TYPE_SPLASH;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_DIALOG)
        {
            windowType = EWMH_WINDOW_TYPE_DIALOG;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU)
        {
            windowType = EWMH_WINDOW_TYPE_DROPDOWN_MENU;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU)
        {
            windowType = EWMH_WINDOW_TYPE_POPUP_MENU;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_TOOLTIP)
        {
            windowType = EWMH_WINDOW_TYPE_TOOLTIP;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION)
        {
            windowType = EWMH_WINDOW_TYPE_NOTIFICATION;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_COMBO)
        {
            windowType = EWMH_WINDOW_TYPE_COMBO;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_DND)
        {
            windowType = EWMH_WINDOW_TYPE_DND;
        }
        else if (win_type.atoms[0] == EWMH._NET_WM_WINDOW_TYPE_NORMAL)
        {
            windowType = EWMH_WINDOW_TYPE_NORMAL;
        }
        else
        {
            windowType = EWMH_WINDOW_TYPE_UNKNOWN;
        }
    }

    return windowType;
}

std::vector<WMNormalHints> EWMHManager::getWMNormalHints(xcb_connection_t* connection, xcb_window_t window) {
    std::vector<WMNormalHints> hintsList;
    xcb_get_property_cookie_t cookie;
    xcb_get_property_reply_t* reply;

    cookie = xcb_get_property(connection, 0, window, XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS, 0, 0xFFFF);
    reply = xcb_get_property_reply(connection, cookie, nullptr);

    if (reply && xcb_get_property_value_length(reply) >= sizeof(WMNormalHints)) {
        const int numHints = xcb_get_property_value_length(reply) / sizeof(WMNormalHints);
        const WMNormalHints* hintsArray = reinterpret_cast<const WMNormalHints*>(xcb_get_property_value(reply));

        for (int i = 0; i < numHints; i++) {
            hintsList.push_back(hintsArray[i]);
        }
    } else {
        // Remplissez avec des valeurs par défaut ou de gestion d'erreur
    }

    free(reply);
    return hintsList;
}

void EWMHManager::addAllowedActionsAtomToRootWindow(std::vector<xcb_atom_t> atoms)
{
    xcb_ewmh_set_supported(&EWMH, 0, atoms.size(), atoms.data());
    xcb_flush(connection);
}

void EWMHManager::setActiveWindow(xcb_window_t window)
{
    xcb_ewmh_set_active_window(&EWMH, 0, window);
    xcb_flush(connection);
}

Window EWMHManager::getWindow(xcb_window_t window)
{
    xcb_window_t rootWindow = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
    
    // Si la fenêtre est la fenêtre racine, renvoyez nullptr.
    if (window == rootWindow) {
        return Window();
    }

    ConfigurationWindow configuration;
    configuration.windowId = window;
    // je récupère les informations sur la fenêtre
    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(connection, xcb_get_geometry(connection, window), NULL);
    xcb_get_window_attributes_reply_t *attributes = xcb_get_window_attributes_reply(connection, xcb_get_window_attributes(connection, window), NULL);
    configuration.x = geometry->x;
    configuration.y = geometry->y;
    configuration.width = geometry->width;
    configuration.height = geometry->height;
    configuration.borderWidth = geometry->border_width;
    configuration.border = attributes->override_redirect;
    configuration.displayScreen = 0;
    // je récupère la classe de la fenêtre
    xcb_get_property_cookie_t cookie = xcb_get_property(connection, 0, window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 0, 100);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(connection, cookie, NULL);
    configuration.windowClass = XCB_ATOM_STRING ? std::string((char *)xcb_get_property_value(reply)) : "";
    if (reply)
    {
        if (reply->type == XCB_ATOM_STRING)
        {
            char *windowClass = (char *)xcb_get_property_value(reply);
            configuration.windowClass = std::string(windowClass);
        }
        free(reply);
    }
    // je récupère le nom de la fenêtre
    cookie = xcb_get_property(connection, 0, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100);
    reply = xcb_get_property_reply(connection, cookie, NULL);
    configuration.windowName = XCB_ATOM_STRING ? std::string((char *)xcb_get_property_value(reply)) : "";
    free(reply);

    Window windowInfo;
    windowInfo.normalConfiguration = configuration;
    windowInfo.type = getWindowType(window);
    windowInfo.normalHints = getWMNormalHints(connection, window);
    // Je vérivie si la fenêtre est en override redirect
    xcb_get_window_attributes_cookie_t cookieAttributes = xcb_get_window_attributes(connection, window);
    xcb_get_window_attributes_reply_t *replyAttributes = xcb_get_window_attributes_reply(connection, cookieAttributes, NULL);
    windowInfo.isOverrideRedirect = replyAttributes->override_redirect;
    free(replyAttributes);
    // Je vérifie si la fenêtre est une fenêtre enfant transiente d'une autre fenêtre
    xcb_get_property_cookie_t cookieTransientFor = xcb_get_property(connection, 0, window, XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 100);
    xcb_get_property_reply_t *replyTransientFor = xcb_get_property_reply(connection, cookieTransientFor, NULL);
    // Si elle est une fenêtre enfant transiente alors je récupère l'identifiant de la fenêtre parent
    if(replyTransientFor->type == XCB_ATOM_WINDOW)
    {
        xcb_window_t *windowTransientFor = (xcb_window_t *)xcb_get_property_value(replyTransientFor);
        windowInfo.transientFor = *windowTransientFor;
    }
    free(replyTransientFor);

    // Je récupère le PID de la fenêtre
    xcb_get_property_cookie_t pidCookie = xcb_get_property(connection, 0, window, EWMH._NET_WM_PID, XCB_ATOM_CARDINAL, 0, 1);
    xcb_get_property_reply_t *pidReply = xcb_get_property_reply(connection, pidCookie, NULL);
    if (pidReply && (pidReply->type == XCB_ATOM_CARDINAL) && (pidReply->format == 32) && (xcb_get_property_value_length(pidReply) == sizeof(uint32_t))) {
        uint32_t *pidValue = (uint32_t *)xcb_get_property_value(pidReply);
        windowInfo.pid = static_cast<int>(*pidValue);
    } else {
        windowInfo.pid = 0; // Aucun PID trouvé ou la propriété n'existe pas.
    }

    if (pidReply) {
        free(pidReply);
    }

    // Ici, nous récupérons les _NET_WM_STATE de la fenêtre et adaptons la configuration
    xcb_get_property_cookie_t stateCookie = xcb_get_property(connection, 0, window, EWMH._NET_WM_STATE, XCB_ATOM_ATOM, 0, UINT32_MAX);
    xcb_get_property_reply_t *stateReply = xcb_get_property_reply(connection, stateCookie, NULL);

    if (stateReply && stateReply->type == XCB_ATOM_ATOM && stateReply->format == 32) {
        xcb_atom_t *atomList = static_cast<xcb_atom_t*>(xcb_get_property_value(stateReply));
        int numAtoms = xcb_get_property_value_length(stateReply) / sizeof(xcb_atom_t);

        for (int i = 0; i < numAtoms; i++) {
            EWMHSTATES state =atomToEWMHSTATES(atomList[i]);
            windowInfo.atoms.push_back(state);
        }
    }

    if (stateReply) {
        free(stateReply);
    }


    return windowInfo;
}

void EWMHManager::printWindowProperties(Window window)
{
    std::cout << "Propriétés de la fenêtre " << window.normalConfiguration.windowId << std::endl;
    std::cout << "\tNom de la fenêtre: " << window.normalConfiguration.windowName << std::endl;
    std::cout << "\tClasse de la fenêtre: " << window.normalConfiguration.windowClass << std::endl;
    switch (window.type)
    {
        case 0:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_DESKTOP" << std::endl;
        break;
        case 1:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_DOCK" << std::endl;
        break;
        case 2:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_TOOLBAR" << std::endl;
        break;
        case 3:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_MENU" << std::endl;
        break;
        case 4:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_UTILITY" << std::endl;
        break;
        case 5:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_SPLASH" << std::endl;
        break;
        case 6:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_DIALOG" << std::endl;
        break;
        case 7:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_NORMAL" << std::endl;
        break;
        case 8:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_DROPDOWN_MENU" << std::endl;
        break;
        case 9:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_POPUP_MENU" << std::endl;
        break;
        case 10:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_TOOLTIP" << std::endl;
        break;
        case 11:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_NOTIFICATION" << std::endl;
        break;
        case 12:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_COMBO" << std::endl;
        break;
        case 13:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_DND" << std::endl;
        break;
        case 14:
            std::cout << "\tType de la fenêtre: EWMH_WINDOW_TYPE_UNKNOWN" << std::endl;
        break;
    }
    std::cout << "\tPosition de la fenêtre: " << window.normalConfiguration.x << " " << window.normalConfiguration.y << std::endl;
    std::cout << "\tTaille de la fenêtre: " << window.normalConfiguration.width << " " << window.normalConfiguration.height << std::endl;
    std::cout << "\tBordure de la fenêtre: " << window.normalConfiguration.borderWidth << std::endl;
    // J'affiche les états de la fenêtre
    std::cout << "\tEtats de la fenêtre:\n ";
    for(EWMHSTATES state: window.atoms)
    {
        switch (state)
        {
            case 0:
                std::cout << "\t\tEWMH_STATE_MODAL \n";
            break;
            case 1:
                std::cout << "\t\tEWMH_STATE_STICKY \n";
            break;
            case 2:
                std::cout << "\t\tEWMH_STATE_MAXIMIZED_VERT \n";
            break;
            case 3:
                std::cout << "\t\tEWMH_STATE_MAXIMIZED_HORZ \n";
            break;
            case 4:
                std::cout << "\t\tEWMH_STATE_SHADED \n";
            break;
            case 5:
                std::cout << "\t\tEWMH_STATE_SKIP_TASKBAR \n";
            break;
            case 6:
                std::cout << "\t\tEWMH_STATE_SKIP_PAGER \n";
            break;
            case 7:
                std::cout << "\t\tEWMH_STATE_HIDDEN \n";
            break;
            case 8:
                std::cout << "\t\tEWMH_STATE_FULLSCREEN \n";
            break;
            case 9:
                std::cout << "\t\tEWMH_STATE_ABOVE \n";
            break;
            case 10:
                std::cout << "\t\tEWMH_STATE_BELOW \n";
            break;
            case 11:
                std::cout << "\t\tEWMH_STATE_DEMANDS_ATTENTION \n";
            break;
            case 12:
                std::cout << "\t\tEWMH_STATE_FOCUSED \n";
            break;
            case 13:
                std::cout << "\t\tEWMH_STATE_NORMAL \n";
            break;
        }
    }

    // J'affiche les hints WM_NORMAL_HINTS
    std::cout << "\tHints WM_NORMAL_HINTS: \n";
    for(WMNormalHints hint: window.normalHints)
    {
        std::cout << "\t\tflags: " << hint.flags << std::endl;
        std::cout << "\t\tmin_width: " << hint.min_width << std::endl;
        std::cout << "\t\tmin_height: " << hint.min_height << std::endl;
        std::cout << "\t\tmax_width: " << hint.max_width << std::endl;
        std::cout << "\t\tmax_height: " << hint.max_height << std::endl;
        std::cout << "\t\tbase_width: " << hint.base_width << std::endl;
        std::cout << "\t\tbase_height: " << hint.base_height << std::endl;
        std::cout << "\t\twidth_inc: " << hint.width_inc << std::endl;
        std::cout << "\t\theight_inc: " << hint.height_inc << std::endl;
        std::cout << "\t\tmin_aspect_num: " << hint.min_aspect_num << std::endl;
        std::cout << "\t\tmin_aspect_den: " << hint.min_aspect_den << std::endl;
        std::cout << "\t\tmax_aspect_num: " << hint.max_aspect_num << std::endl;
        std::cout << "\t\tmax_aspect_den: " << hint.max_aspect_den << std::endl;
        std::cout << "\t\twin_gravity: " << hint.win_gravity << std::endl;
    }
}

EWMHSTATES EWMHManager::atomToEWMHSTATES(xcb_atom_t atom)
{
    if(atom == EWMH._NET_WM_STATE_MODAL) return EWMH_STATE_MODAL;
    else if(atom== EWMH._NET_WM_STATE_BELOW) return EWMH_STATE_BELOW;
    else if(atom== EWMH._NET_WM_STATE_ABOVE) return EWMH_STATE_ABOVE;
    else if(atom== EWMH._NET_WM_STATE_DEMANDS_ATTENTION) return EWMH_STATE_DEMANDS_ATTENTION;
    else if(atom==EWMH._NET_WM_STATE_FULLSCREEN) return EWMH_STATE_FULLSCREEN;
    else if(atom==EWMH._NET_WM_STATE_HIDDEN) return EWMH_STATE_HIDDEN;
    else if(atom==EWMH._NET_WM_STATE_MAXIMIZED_HORZ) return EWMH_STATE_MAXIMIZED_HORZ;
    else if(atom==EWMH._NET_WM_STATE_MAXIMIZED_VERT) return EWMH_STATE_MAXIMIZED_VERT;
    else if(atom==EWMH._NET_WM_STATE_MODAL) return EWMH_STATE_MODAL;
    else if(atom==EWMH._NET_WM_STATE_SHADED) return EWMH_STATE_SHADED;
    else if(atom==EWMH._NET_WM_STATE_SKIP_PAGER) return EWMH_STATE_SKIP_PAGER;
    else if(atom==EWMH._NET_WM_STATE_SKIP_TASKBAR) return EWMH_STATE_SKIP_TASKBAR;
    else if(atom==EWMH._NET_WM_STATE_STICKY) return EWMH_STATE_STICKY;
    else return EWMH_STATE_NORMAL;
}


