#include "xcbManager.h"

XcbManager::XcbManager(LogManager &log)
{
    this->logManager=&log;
    // On se connecte au serveur X
    logManager->addLog(Log("Connexion au serveur X",LogSeverity::Info,"XcbManager->XcbManager"));
    connection = xcb_connect(NULL, NULL);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    info = new DisplayInfo(connection);
    if (xcb_connection_has_error(connection))
    {
        logManager->addLog(Log("Impossible de se connecter au serveur X",LogSeverity::Error,"XcbManager->XcbManager"));
        exit(-1);
    }

    // je crée le masque d'évènement pour la fenêtre racine
    const uint32_t select_input_val[] =
        {
            XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
            XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW |
            XCB_EVENT_MASK_STRUCTURE_NOTIFY |
            XCB_EVENT_MASK_PROPERTY_CHANGE |
            XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_KEY_PRESS};

    // Je capture la touche Windows pour afficher l'interface de configuration
    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(connection);
    xcb_keycode_t *keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_Super_L);
    xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    // Je capture la touche f12 pour mettre la fenêtre sélectionnée en plein écran ou la remettre à sa taille d'origine
    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_F12);
    xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    // Je capture la touche numlock
    keycodesPtr = xcb_key_symbols_get_keycode(keysyms, XK_Num_Lock);
    xcb_grab_key(connection, 1, screen->root, XCB_MOD_MASK_ANY, keycodesPtr[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    free(keycodesPtr);
    xcb_key_symbols_free(keysyms);

    xcb_change_window_attributes(connection,
                                 screen->root,
                                 XCB_CW_EVENT_MASK, select_input_val);

    if (xcb_poll_for_event(connection) != NULL)
    {
        logManager->addLog(Log("Un autre Window Manager fonctionne",LogSeverity::Error,"XcbManager->XcbManager"));
        exit(1);
    };

    xcb_flush(connection);    

    ewmhManager = new EWMHManager(connection);
}

XcbManager::~XcbManager() // Définition du destructeur
{
    // On se déconnecte du serveur X
    logManager->addLog(Log("Déconnexion du serveur X",LogSeverity::Warning,"XcbManager->~XcbManager"));
    xcb_disconnect(connection);;
    delete info;
    delete ewmhManager;
}

EWMHManager *XcbManager::getEWMHManager()
{
    return ewmhManager;
}

void XcbManager::flush()
{
    xcb_flush(connection);
}

xcb_connection_t *XcbManager::getConnection()
{
    return connection;
}

xcb_screen_t *XcbManager::getScreen()
{
    return screen;
}

int XcbManager::getDisplayScreen(const ConfigurationWindow configuration)
{
    // je récupère les informations sur les écrans avec la classe DisplayInfo
    const std::vector<Display> screens = info->getScreens();
    int maxIntersection = 0;
    int selectedScreenIndex = 0;
    for (int i = 0; i < screens.size(); i++)
    {
        // je calcule la surface de l'intersection entre la fenêtre et l'écran
        const int x1 = std::max(configuration.x, screens[i].x);
        const int y1 = std::max(configuration.y, screens[i].y);
        const int x2 = std::min(configuration.x + configuration.width, screens[i].x + screens[i].width);
        const int y2 = std::min(configuration.y + configuration.height, screens[i].y + screens[i].height);
        int intersection = (x2 - x1) * (y2 - y1);

        if (intersection > maxIntersection)
        {
            maxIntersection = intersection;
            selectedScreenIndex = i;
        }
    }
    return selectedScreenIndex;
}


std::vector<Display> XcbManager::getDisplayConfiguration()
{
    return info->getScreens();
}

std::string XcbManager::getDisplayConfigurationForSocket()
{
    return microWM::serializeDisplays(info->getScreens());
}

Window XcbManager::getWindow(xcb_window_t window)
{
    Window windowInfo = ewmhManager->getWindow(window);
    ConfigurationWindow configuration = windowInfo.normalConfiguration;
    configuration.displayScreen = getDisplayScreen(configuration);
    configuration.relativeX = configuration.x - info->getScreens()[configuration.displayScreen].x;
    configuration.relativeY = configuration.y - info->getScreens()[configuration.displayScreen].y;
    windowInfo.normalConfiguration = configuration;
    // Je regarde si la fenêtre est en plein écran
    std::vector<EWMHSTATES> atoms;
    if (configuration.width == info->getScreens()[configuration.displayScreen].width && configuration.height == info->getScreens()[configuration.displayScreen].height)
    {
        atoms.push_back(EWMH_STATE_FULLSCREEN);
        // J'ajoute l'atome _NET_WM_STATE_FULLSCREEN à la fenêtre
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_FULLSCREEN);
    }

    windowInfo.atoms = atoms;

    return windowInfo;
}

void XcbManager::centerWindow(xcb_window_t windowA, xcb_window_t windowB)
{
    ConfigurationWindow configurationA;
    ConfigurationWindow configurationB;
    if(windows.find(windowA)!=windows.end())
    {
        configurationA=windows[windowA].normalConfiguration;
    }
    else
    {
        // je récupère les informations sur les fenêtres
        configurationA = getWindow(windowA).normalConfiguration;
    }
    if(windows.find(windowB)!=windows.end())
    {
            configurationB=getWindow(windowB).normalConfiguration;
    }
    else
    {
            configurationB = getWindow(windowB).normalConfiguration;
    }
    // je centre la fenêtre A par rapport à la fenêtre B
    configurationA.x = configurationB.x + (configurationB.width - configurationA.width) / 2;
    configurationA.y = configurationB.y + (configurationB.height - configurationA.height) / 2;
    // je met a jour la fenêtre A
    int32_t values[2]={static_cast<int32_t>(configurationA.x), static_cast<int32_t>(configurationA.y)};
    // J'enregistre la nouvelle position de la fenêtre dans windows
    if(windows.find(windowA)!=windows.end())
    {
        windows[windowA].normalConfiguration = configurationA;
    }
    xcb_configure_window(connection, windowA, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(connection);
}


void XcbManager::xcb_aux_sync(xcb_connection_t *c)
{
    free(xcb_get_input_focus_reply(c, xcb_get_input_focus(c), NULL));
}

void XcbManager::applyGeometryConfiguration(ConfigurationWindow configuration, bool rewrite)
{
    if(configuration.windowId != 0)
    {
        logManager->addLog(Log("Application d'une configuration géomérique pour la fenêtre avec l'ID: " + std::to_string(configuration.windowId), LogSeverity::Info, "XcbManager->applyGeometryConfiguration"));

        // Configure la taille de la fenêtre
        uint32_t values[2] = {
            static_cast<uint32_t>(configuration.width),
            static_cast<uint32_t>(configuration.height)
        };

        xcb_void_cookie_t cookie_size = xcb_configure_window_checked(connection, configuration.windowId, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

        // Vérification des erreurs pour la configuration de la taille
        xcb_generic_error_t *error = xcb_request_check(connection, cookie_size);
        if (error)
        {
            logManager->addLog(Log("Erreur lors de la configuration de la taille de la fenêtre: " + std::to_string(error->error_code), LogSeverity::Error, "XcbManager->applyGeometryConfiguration"));
            free(error);
        }

        // Configure la position de la fenêtre
        values[0] = static_cast<uint32_t>(configuration.x);
        values[1] = static_cast<uint32_t>(configuration.y);

        xcb_void_cookie_t cookie_position = xcb_configure_window_checked(connection, configuration.windowId, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

        // Vérification des erreurs pour la configuration de la position
        error = xcb_request_check(connection, cookie_position);
        if (error)
        {
            logManager->addLog(Log("Erreur lors de la configuration de la position de la fenêtre: " + std::to_string(error->error_code), LogSeverity::Error, "XcbManager->applyGeometryConfiguration"));
            free(error);
        }

        // Synchronisation après la configuration pour s'assurer que les changements sont appliqués
        xcb_aux_sync(connection);

        if(rewrite)
        {
            // Enregistrement de la nouvelle position de la fenêtre dans windows
            if(windows.find(configuration.windowId)!=windows.end())
            {
                windows[configuration.windowId].normalConfiguration = configuration;
                // Si la configuration n'est pas en plein écran, suppression de l'état plein écran de la fenêtre
                if(configuration.width != info->getScreens()[configuration.displayScreen].width || configuration.height != info->getScreens()[configuration.displayScreen].height)
                {
                    auto &atoms = windows[configuration.windowId].atoms;
                    atoms.erase(std::remove(atoms.begin(), atoms.end(), EWMH_STATE_FULLSCREEN), atoms.end());
                }
            }
        }

        xcb_flush(connection);
    }
}

void XcbManager::applyGeometryConfiguration(std::string jsonConfigurationWindow)
{
    applyGeometryConfiguration(deserializeConfigurationWindow(jsonConfigurationWindow),true);
}

void XcbManager::toggleFullscreen(xcb_window_t window)
{
    // Je regardes si la fenêtre est déjà en plein écran dans windows
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN) != windows[window].atoms.end())
    {
        Window toWindow = windows[window];
        applyGeometryConfiguration(toWindow.normalConfiguration,true);
        // Je supprime l'état plein écran de la fenêtre
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN), windows[window].atoms.end()); 
        // Je supprime l'atome _NET_WM_STATE_FULLSCREEN de la fenêtre
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_FULLSCREEN);
    }
    else
    {
        Window toWindow = windows[window];
        toWindow.normalConfiguration.x = info->getScreens()[toWindow.normalConfiguration.displayScreen].x;
        toWindow.normalConfiguration.y = info->getScreens()[toWindow.normalConfiguration.displayScreen].y;
        toWindow.normalConfiguration.width = info->getScreens()[toWindow.normalConfiguration.displayScreen].width;
        toWindow.normalConfiguration.height = info->getScreens()[toWindow.normalConfiguration.displayScreen].height;
        toWindow.normalConfiguration.relativeX = 0;
        toWindow.normalConfiguration.relativeY = 0;
        applyGeometryConfiguration(toWindow.normalConfiguration,false);
        // J'ajoute l'état plein écran à la fenêtre
        windows[window].atoms.push_back(EWMH_STATE_FULLSCREEN);
        // J'ajoute l'atome _NET_WM_STATE_FULLSCREEN à la fenêtre
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_FULLSCREEN);        
    }
}

void XcbManager::addFullscreen(xcb_window_t window)
{
    // Je regardes si la fenêtre est déjà en plein écran dans windows
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN) == windows[window].atoms.end())
    {
        Window toWindow = windows[window];
        toWindow.normalConfiguration.x = info->getScreens()[toWindow.normalConfiguration.displayScreen].x;
        toWindow.normalConfiguration.y = info->getScreens()[toWindow.normalConfiguration.displayScreen].y;
        toWindow.normalConfiguration.width = info->getScreens()[toWindow.normalConfiguration.displayScreen].width;
        toWindow.normalConfiguration.height = info->getScreens()[toWindow.normalConfiguration.displayScreen].height;
        toWindow.normalConfiguration.relativeX = 0;
        toWindow.normalConfiguration.relativeY = 0;
        applyGeometryConfiguration(toWindow.normalConfiguration,false);
        // J'ajoute l'état plein écran à la fenêtre
        windows[window].atoms.push_back(EWMH_STATE_FULLSCREEN);
        // J'ajoute l'atome _NET_WM_STATE_FULLSCREEN à la fenêtre
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_FULLSCREEN);        
    }
}

void XcbManager::removeFullscreen(xcb_window_t window)
{
    // Je regardes si la fenêtre est déjà en plein écran dans windows
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN) != windows[window].atoms.end())
    {
        Window toWindow = windows[window];
        applyGeometryConfiguration(toWindow.normalConfiguration,true);
        // Je supprime l'état plein écran de la fenêtre
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN), windows[window].atoms.end()); 
        // Je supprime l'atome _NET_WM_STATE_FULLSCREEN de la fenêtre
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_FULLSCREEN);
    }
}

void XcbManager::addAbove(xcb_window_t window)
{
    // Si la fenêtre n'est pas déjà au dessus
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_ABOVE) == windows[window].atoms.end())
    {   windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_ABOVE);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_ABOVE);
        restack_windows();
    }      
}

void XcbManager::removeAbove(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_FULLSCREEN) != windows[window].atoms.end())
    {
        // Je supprime l'état above
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_ABOVE), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_ABOVE);
        restack_windows();
    }
}

void XcbManager::toggleAbove(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_ABOVE) == windows[window].atoms.end())
    {   windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_ABOVE);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_ABOVE);
        restack_windows();
    }
    else
    {
         // Je supprime l'état above
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_ABOVE), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_ABOVE);
        restack_windows();       
    }    
}

void XcbManager::addBelow(xcb_window_t window)
{
    // Si la fenêtre n'est pas déjà en dessous
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_BELOW) == windows[window].atoms.end())
    {   
        windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_BELOW);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_BELOW);
        restack_windows();
    }      
}

void XcbManager::removeBelow(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_BELOW) != windows[window].atoms.end())
    {
        // Je supprime l'état below
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_BELOW), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_BELOW);
        restack_windows();
    }
}

void XcbManager::toggleBelow(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_BELOW) == windows[window].atoms.end())
    {   
        windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_BELOW);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_BELOW);
        restack_windows();
    }
    else
    {
         // Je supprime l'état below
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_BELOW), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_BELOW);
        restack_windows();       
    }    
}

void XcbManager::addHidden(xcb_window_t window)
{
    // Si la fenêtre n'est pas déjà cachée
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_HIDDEN) == windows[window].atoms.end())
    {   
        windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_HIDDEN);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_HIDDEN);
        xcb_unmap_window(connection,window);
        xcb_flush(connection);
    }      
}

void XcbManager::removeHidden(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_HIDDEN) != windows[window].atoms.end())
    {
        // Je supprime l'état hidden
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_HIDDEN), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_HIDDEN);
        xcb_map_window(connection,window);
        xcb_flush(connection);
    }
}

void XcbManager::toggleHidden(xcb_window_t window)
{
   if (windows.find(window) != windows.end())
    if (std::find(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_HIDDEN) == windows[window].atoms.end())
    {
        windows[window].atoms.push_back(EWMHSTATES::EWMH_STATE_HIDDEN);
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 1, &ewmhManager->EWMH._NET_WM_STATE_HIDDEN);
        xcb_unmap_window(connection,window);
    }
    else
    {
        // Je supprime l'état hidden
        windows[window].atoms.erase(std::remove(windows[window].atoms.begin(), windows[window].atoms.end(), EWMH_STATE_HIDDEN), windows[window].atoms.end());   
        xcb_ewmh_set_wm_state(&ewmhManager->EWMH, window, 0, &ewmhManager->EWMH._NET_WM_STATE_HIDDEN);
        xcb_map_window(connection,window);
    }
    xcb_flush(connection);
}

xcb_window_t XcbManager::getActiveWindow()
{
    // Je parcours windows et je récupère la fenêtre qui a le state EWMH_STATE_FOCUSED 
    for (auto const& window : windows)
    {
        if (std::find(window.second.atoms.begin(), window.second.atoms.end(), EWMH_STATE_FOCUSED) != window.second.atoms.end()&&window.first!=screen->root&&window.second.type!=EWMHtype::EWMH_WINDOW_TYPE_DESKTOP&&window.second.type!=EWMHtype::EWMH_WINDOW_TYPE_DOCK)
        {
            return window.first;
        }
    }
    return 0; 
}

void XcbManager::setDockHeight(uint32_t height)
{
    Window tWindow;
    for(auto sWindow:windows)
    {
        if(sWindow.second.type==EWMHtype::EWMH_WINDOW_TYPE_DOCK)
        tWindow=sWindow.second;
    }
    tWindow.normalConfiguration.height=height;
    applyGeometryConfiguration(tWindow.normalConfiguration,true);
    restack_windows();
}

void XcbManager::setActiveWindow(xcb_window_t window)
{
    if (windows.find(window) != windows.end())
    {
        Window &thisEwmhWindow=windows[window];
        Window &currentFocusedWindow=windows[getActiveWindow()];
        // je vérifie si la fenêtre précédente n'est pas modale
        bool isModal{false};
        for(EWMHSTATES state:currentFocusedWindow.atoms)
        {
            if(state==EWMHSTATES::EWMH_STATE_MODAL)
            {
                isModal=true;
            }
        }
        // Si la fenêtre précédente n'est pas modale
        if(!isModal)
        {
            ewmhManager->setActiveWindow(window);
            // Je donne le focus à la fenêtre
            xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, window, XCB_CURRENT_TIME);
            // Je donne l'atôme active window
            xcb_ewmh_set_active_window(&ewmhManager->EWMH, 0, window);
            // Je parcours toutes les fenêtres gérées par le gestionnaire de fenêtres pour mettre l'atome _NET_WM_STATE_FOCUSED sur la fenêtre qui a le focus sauf si desktop
            for (auto it = windows.begin(); it != windows.end(); it++)
            {
                // Si la fenêtre est la fenêtre qui a le focus
                if(it->first==window)
                {
                    // Si la fenêtre n'a pas l'atome _NET_WM_STATE_FOCUSED
                    if(std::find(it->second.atoms.begin(), it->second.atoms.end(), EWMH_STATE_FOCUSED)==it->second.atoms.end())
                    {
                        // J'ajoute l'atome _NET_WM_STATE_FOCUSED
                        it->second.atoms.push_back(EWMH_STATE_FOCUSED);
                    }
                }
                // Si la fenêtre n'est pas la fenêtre qui a le focus
                else
                {
                    // Si la fenêtre a l'atome _NET_WM_STATE_FOCUSED
                    if(std::find(it->second.atoms.begin(), it->second.atoms.end(), EWMH_STATE_FOCUSED)!=it->second.atoms.end())
                    {
                        // Je supprime l'atome _NET_WM_STATE_FOCUSED
                        it->second.atoms.erase(std::remove(it->second.atoms.begin(), it->second.atoms.end(), EWMH_STATE_FOCUSED), it->second.atoms.end());
                    }
                }
            }
        }
        xcb_flush(connection);
    }
}

void XcbManager::setWindowOnRatio(xcb_window_t window, double xRatio, double yRatio, double widthRatio, double heightRatio)
{
    if (windows.find(window) != windows.end())
    {
        ConfigurationWindow configuration = windows[window].normalConfiguration;
        Display display = info->getScreens()[configuration.displayScreen];

        configuration.x = display.x + static_cast<int>(static_cast<double>(display.width) * xRatio);
        configuration.y = display.y + static_cast<int>(static_cast<double>(display.height) * yRatio);
        configuration.width = static_cast<int>(static_cast<double>(display.width) * widthRatio);
        configuration.height = static_cast<int>(static_cast<double>(display.height) * heightRatio);
        configuration.relativeX = configuration.x - display.x;
        configuration.relativeY = configuration.y - display.y;
        applyGeometryConfiguration(configuration,true);
    }
}

Window XcbManager::getWindowOnRatio(xcb_window_t window, double xRatio, double yRatio, double widthRatio, double heightRatio)
{
    ConfigurationWindow configuration = windows[window].normalConfiguration;
    Display display = info->getScreens()[configuration.displayScreen];

    configuration.x = display.x + static_cast<int>(static_cast<double>(display.width) * xRatio);
    configuration.y = display.y + static_cast<int>(static_cast<double>(display.height) * yRatio);
    configuration.width = static_cast<int>(static_cast<double>(display.width) * widthRatio);
    configuration.height = static_cast<int>(static_cast<double>(display.height) * heightRatio);
    configuration.relativeX = configuration.x - display.x;
    configuration.relativeY = configuration.y - display.y;
    Window windowInfo = windows[window];
    windowInfo.normalConfiguration = configuration;
    return windowInfo;
}

void XcbManager::setWindowOnTopLeft(xcb_window_t window)
{
   setWindowOnRatio(window, 0.0, 0.0, 0.5, 0.5);
}

void XcbManager::setWindowOnTopRight(xcb_window_t window)
{
    setWindowOnRatio(window, 0.5, 0.0, 0.5, 0.5);
}

void XcbManager::setWindowOnBottomLeft(xcb_window_t window)
{
    setWindowOnRatio(window, 0.0, 0.5, 0.5, 0.5);
}

void XcbManager::setWindowOnBottomRight(xcb_window_t window)
{
    setWindowOnRatio(window, 0.5, 0.5, 0.5, 0.5);
}

void XcbManager::setWindowOnLeft(xcb_window_t window)
{
    setWindowOnRatio(window, 0.0, 0.0, 0.5, 1.0);
}

void XcbManager::setWindowOnRight(xcb_window_t window)
{
    setWindowOnRatio(window, 0.5, 0.0, 0.5, 1.0);
}

void XcbManager::setWindowOnTop(xcb_window_t window)
{
    setWindowOnRatio(window, 0.0, 0.0, 1.0, 0.5);
}

void XcbManager::setWindowOnBottom(xcb_window_t window)
{
    setWindowOnRatio(window, 0.0, 0.5, 1.0, 0.5);
}

void XcbManager::setWindowOnNextScreen(xcb_window_t window)
{
    Window toWindow=windows[window];
    // Je récupère l'index de l'écran sur lequel est la fenêtre
    int screenIndex =toWindow.normalConfiguration.displayScreen;
    // Je récupère le nombre d'écrans
    int screenCount = info->getScreens().size();
    // Je récupère l'index de l'écran suivant
    int nextScreenIndex = (screenIndex + 1) % screenCount;
    // Je récupère la configuration de l'écran suivant
    Display nextScreen = info->getScreens()[nextScreenIndex];
    // Je calcule les ratios de taille et de position entre l'écran actuel et l'écran suivant
    double ratioWidth = static_cast<double>(nextScreen.width) / static_cast<double>(info->getScreens()[screenIndex].width);
    double ratioHeight = static_cast<double>(nextScreen.height) / static_cast<double>(info->getScreens()[screenIndex].height);
    double ratioX = static_cast<double>(nextScreen.x - info->getScreens()[screenIndex].x) / static_cast<double>(info->getScreens()[screenIndex].width);
    double ratioY = static_cast<double>(nextScreen.y - info->getScreens()[screenIndex].y) / static_cast<double>(info->getScreens()[screenIndex].height);
    // Je met à jour la configuration de la fenêtre
    toWindow.normalConfiguration.x = info->getScreens()[nextScreenIndex].x + static_cast<int>(static_cast<double>(toWindow.normalConfiguration.x - info->getScreens()[screenIndex].x) * ratioWidth);
    toWindow.normalConfiguration.y = info->getScreens()[nextScreenIndex].y + static_cast<int>(static_cast<double>(toWindow.normalConfiguration.y - info->getScreens()[screenIndex].y) * ratioHeight);
    toWindow.normalConfiguration.width = static_cast<int>(static_cast<double>(toWindow.normalConfiguration.width) * ratioWidth);
    toWindow.normalConfiguration.height = static_cast<int>(static_cast<double>(toWindow.normalConfiguration.height) * ratioHeight);
    toWindow.normalConfiguration.relativeX = toWindow.normalConfiguration.x - info->getScreens()[nextScreenIndex].x;
    toWindow.normalConfiguration.relativeY = toWindow.normalConfiguration.y - info->getScreens()[nextScreenIndex].y;
    toWindow.normalConfiguration.displayScreen = nextScreenIndex;
    applyGeometryConfiguration(toWindow.normalConfiguration,true);
}

void XcbManager::setToDesktop(int id)
{
    xcb_ewmh_set_wm_window_type(&ewmhManager->EWMH, id, 1, &ewmhManager->EWMH._NET_WM_WINDOW_TYPE_DESKTOP);
    windows[id].type=EWMH_WINDOW_TYPE_DESKTOP;
    restack_windows();
}

void XcbManager::setToDock(int id)
{
    xcb_ewmh_set_wm_window_type(&ewmhManager->EWMH, id, 1, &ewmhManager->EWMH._NET_WM_WINDOW_TYPE_DOCK);
    windows[id].type=EWMH_WINDOW_TYPE_DOCK;   
    restack_windows();
}


void XcbManager::setWindowOpacity(xcb_window_t window, uint8_t opacity)
{;
    // Je crée l'atome _NET_WM_WINDOW_OPACITY si il n'existe pas
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_WINDOW_OPACITY"), "_NET_WM_WINDOW_OPACITY");
    xcb_intern_atom_reply_t *atomReply = xcb_intern_atom_reply(connection, atomCookie, NULL);
    xcb_atom_t atom = atomReply->atom;
    free(atomReply);
    // Je convertis l'opacité en entier
    uint32_t opacityValue = static_cast<uint32_t>(opacity);
    // Je met à jour la propriété _NET_WM_WINDOW_OPACITY de la fenêtre
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, atom, XCB_ATOM_CARDINAL, 32, 1, &opacityValue);
    xcb_flush(connection);
}

void XcbManager::applyGeometryAndOpacity(std::vector<Window> windows)
{
    for (auto const& window : windows)
    {
        // J'applique la géométrie de la fenêtre
        int32_t values[4]={static_cast<int32_t>(window.normalConfiguration.x), static_cast<int32_t>(window.normalConfiguration.y), static_cast<int32_t>(window.normalConfiguration.width), static_cast<int32_t>(window.normalConfiguration.height)};
        xcb_configure_window(connection, window.normalConfiguration.windowId, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
        // J'applique l'opacité de la fenêtre
        // Je crée l'atome _NET_WM_WINDOW_OPACITY si il n'existe pas
        xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom(connection, 0, strlen("_NET_WM_WINDOW_OPACITY"), "_NET_WM_WINDOW_OPACITY");
        xcb_intern_atom_reply_t *atomReply = xcb_intern_atom_reply(connection, atomCookie, NULL);
        xcb_atom_t atom = atomReply->atom;
        free(atomReply);
        // Je convertis l'opacité en entier
        uint32_t opacityValue = static_cast<uint32_t>(window.normalConfiguration.opacity)<<24;
        // Je met à jour la propriété _NET_WM_WINDOW_OPACITY de la fenêtre
        xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window.normalConfiguration.windowId, atom, XCB_ATOM_CARDINAL, 32, 1, &opacityValue); 


    }
    xcb_flush(connection);
}

std::vector<Window> XcbManager::convertWindowsToVector(std::unordered_map<xcb_window_t,Window> windows)
{
    std::vector<Window> windowsVector;
    for (auto const& window : windows)
    {
        windowsVector.push_back(window.second);
    }
    return windowsVector;
}

xcb_pixmap_t XcbManager::captureWindow(xcb_window_t windowId)
{
  const xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(connection, xcb_get_geometry(connection, windowId), nullptr);
  
  xcb_pixmap_t pixmapId = xcb_generate_id(connection);
  xcb_create_pixmap(connection, screen->root_depth, pixmapId, windowId, geom->width, geom->height);

  xcb_gcontext_t gc = xcb_generate_id(connection);
  xcb_create_gc(connection, gc, pixmapId, 0, nullptr);
  
  xcb_copy_area(connection, windowId, pixmapId, gc, 0, 0, 0, 0, geom->width, geom->height);

  free(geom);
  xcb_free_gc(connection, gc);

  return pixmapId;    
}

std::string XcbManager::getWindowsForSocket()
{
    std::vector<Window> resultWindow;
    for(auto &it:windows)
    {
        resultWindow.push_back(it.second);
    }
    return serializeWindows(resultWindow);
}

void XcbManager::restack_windows()
{
    // Créer des listes pour les différents types de fenêtres
    std::list<Window> desktopWindows{};
    std::list<Window> belowWindows{};
    std::list<Window> normalWindows{};
    std::list<Window> aboveWindows{};
    std::list<Window> dockWindows{};
    std::list<Window> fullscreenWindows{};
    std::list<Window> attentionWindows{};
    std::list<Window> transientFor{};
    std::vector<Window> fWindows= this->convertWindowsToVector(windows);
    std::sort(fWindows.begin(), fWindows.end(), [](const Window &a, const Window &b) {
        return a.order < b.order;
    });   

    for (Window window: fWindows)
    {
        // Ignorer les fenêtres transitoires pour ce tri
        if (window.transientFor==0)
        {
            bool isHidden = false;
            bool isAbove = false;
            bool isBelow = false;
            bool isFullscreen = false;
            bool demandsAttention = false;

            for (EWMHSTATES state : window.atoms)
            {
                switch (state)
                {
                    case EWMH_STATE_HIDDEN:
                        isHidden = true;
                        break;
                    case EWMH_STATE_ABOVE:
                        isAbove = true;
                        break;
                    case EWMH_STATE_BELOW:
                        isBelow = true;
                        break;
                    case EWMH_STATE_FULLSCREEN:
                        isFullscreen = true;
                        break;
                    case EWMH_STATE_DEMANDS_ATTENTION:
                        demandsAttention = true;
                        break;
                    default:
                        break;
                }
            }

            if (!isHidden)
            {
                if (window.type == EWMH_WINDOW_TYPE_DESKTOP)
                {
                    desktopWindows.push_back(window);
                }
                else if (window.type == EWMH_WINDOW_TYPE_DOCK)
                {
                    dockWindows.push_back(window);
                }
                else if (demandsAttention)
                {
                    attentionWindows.push_back(window);
                }
                else if (isFullscreen)
                {
                    fullscreenWindows.push_back(window);
                }
                else if (isAbove)
                {
                    aboveWindows.push_back(window);
                }
                else if (isBelow)
                {
                    belowWindows.push_back(window);
                }
                else 
                {
                    normalWindows.push_back(window);
                }
            }
        }
        else
        {
            transientFor.push_back(window);
            window.normalConfiguration.sibling=window.transientFor;
        }
    }

    // Restack dans l'ordre d'apparence
    std::list<Window> finalStackingOrder;
    finalStackingOrder.splice(finalStackingOrder.end(), desktopWindows);
    finalStackingOrder.splice(finalStackingOrder.end(), belowWindows);
    finalStackingOrder.splice(finalStackingOrder.end(), normalWindows);
    finalStackingOrder.splice(finalStackingOrder.end(), aboveWindows);
    finalStackingOrder.splice(finalStackingOrder.end(), fullscreenWindows);
    finalStackingOrder.splice(finalStackingOrder.end(), dockWindows);    
    finalStackingOrder.splice(finalStackingOrder.end(), attentionWindows);

    xcb_window_t last = screen->root;
    for (Window window : finalStackingOrder)
    {
            if(window.normalConfiguration.windowId!=0)
            {
                u_int32_t values[]={ last, XCB_STACK_MODE_ABOVE };
                xcb_configure_window(connection, window.normalConfiguration.windowId,
                    XCB_CONFIG_WINDOW_SIBLING | XCB_CONFIG_WINDOW_STACK_MODE,
                    values);
                last = window.normalConfiguration.windowId;
            }
    }
    for (Window window:transientFor)
    {
        u_int32_t values[]={ static_cast<u_int32_t> (window.transientFor), XCB_STACK_MODE_ABOVE };
        xcb_configure_window(connection, window.normalConfiguration.windowId,
            XCB_CONFIG_WINDOW_SIBLING | XCB_CONFIG_WINDOW_STACK_MODE,values);
    }

    xcb_flush(connection);
}

void XcbManager::setWindowOnAbove(xcb_window_t window)
{
    if(windows.find(window)!=windows.end())
    {
        countWindow++;
        windows[window].order=countWindow;
        restack_windows();
    }
}

void XcbManager::setWindowOnBellow(xcb_window_t window)
{
    if(windows.find(window)!=windows.end())
    {
        countDownWindow--;
        windows[window].order=countDownWindow;
        restack_windows();
    }
}

bool XcbManager::check_wm_delete_support(xcb_window_t window)
{
    xcb_atom_t wm_protocols_atom;
    xcb_atom_t wm_delete_window_atom;
    xcb_intern_atom_cookie_t protocols_cookie;
    xcb_intern_atom_cookie_t delete_window_cookie;
    xcb_intern_atom_reply_t *protocols_reply;
    xcb_intern_atom_reply_t *delete_window_reply;
    xcb_get_property_cookie_t property_cookie;
    xcb_get_property_reply_t *property_reply;
    bool supported = false;

    protocols_cookie = xcb_intern_atom(connection, 0, 12, "WM_PROTOCOLS");
    protocols_reply = xcb_intern_atom_reply(connection, protocols_cookie, NULL);
    if (!protocols_reply) return false;
    wm_protocols_atom = protocols_reply->atom;
    free(protocols_reply);

    delete_window_cookie = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
    delete_window_reply = xcb_intern_atom_reply(connection, delete_window_cookie, NULL);
    if (!delete_window_reply) return false;
    wm_delete_window_atom = delete_window_reply->atom;
    free(delete_window_reply);

    property_cookie = xcb_get_property(connection, 0, window, wm_protocols_atom, XCB_ATOM_ATOM, 0, 32);
    property_reply = xcb_get_property_reply(connection, property_cookie, NULL);

    if (property_reply) {
        if (xcb_get_property_value_length(property_reply) > 0) {
            xcb_atom_t *protocols = (xcb_atom_t *)xcb_get_property_value(property_reply);
            for (unsigned int i = 0; i < (property_reply->value_len) && !supported; ++i) {
                if (protocols[i] == wm_delete_window_atom) {
                    supported = true;
                }
            }
        }
        free(property_reply);
    }
    return supported;
}

void XcbManager::sendDeleteWindow(xcb_window_t window)
{
    if(check_wm_delete_support(window))
    {
        xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(connection, 0, 12, "WM_PROTOCOLS");
        xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(connection, wm_protocols_cookie, NULL);
        xcb_atom_t wm_protocols = wm_protocols_reply->atom;
        free(wm_protocols_reply);

        xcb_intern_atom_cookie_t wm_delete_window_cookie = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
        xcb_intern_atom_reply_t* wm_delete_window_reply = xcb_intern_atom_reply(connection, wm_delete_window_cookie, NULL);
        xcb_atom_t wm_delete_window = wm_delete_window_reply->atom;
        free(wm_delete_window_reply);

        xcb_client_message_event_t event = {0};
        event.response_type = XCB_CLIENT_MESSAGE;
        event.window = window;
        event.format = 32;
        event.type = wm_protocols; 
        event.data.data32[0] = wm_delete_window;
        event.data.data32[1] = XCB_CURRENT_TIME;

        xcb_send_event(connection, 0, window, XCB_EVENT_MASK_NO_EVENT, (char*)&event);
        xcb_flush(connection);  
    }
    else
    {
        xcb_kill_client(connection, window);
        windows.erase(window);
        restack_windows();
    }  
}

void XcbManager::deleteWindow(xcb_window_t window)
{
    xcb_destroy_window(connection,window);
    windows.erase(window);
    restack_windows();
}