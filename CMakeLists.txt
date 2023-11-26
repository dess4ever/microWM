cmake_minimum_required(VERSION 3.10)

# Nom du projet et langue utilisée
project(microWM LANGUAGES CXX)

# Spécifiez la version du C++ utilisée
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fichiers source
set(SOURCES
    microWM.cpp
    modules/xcbManager.cpp
    modules/XCBEventsManager.cpp
    modules/displayInfo.cpp
    modules/configurationWindow.cpp
    modules/display.cpp
    modules/EWMHManager.cpp
    modules/SocketUnix.cpp
    modules/window.cpp
    modules/logManager.cpp
    modules/log.cpp
    modules/textUtil.cpp
)
set (TEST_SOURCES
    test.cpp
    modules/xcbManager.cpp
    modules/XCBEventsManager.cpp
    modules/displayInfo.cpp
    modules/configurationWindow.cpp
    modules/display.cpp
    modules/EWMHManager.cpp
    modules/SocketUnix.cpp
    modules/window.cpp
    modules/logManager.cpp
    modules/log.cpp
    modules/textUtil.cpp
    )
    

# Définir la cible de sortie et ajouter les fichiers source à celle-ci
add_executable(microWM ${SOURCES})
add_executable(test ${TEST_SOURCES})

# Bibliothèques liées
target_link_libraries(microWM PRIVATE
    png
    xcb
    xcb-randr
    xcb-keysyms
    xcb-ewmh
    X11
)

target_link_libraries(test PRIVATE
    png
    xcb
    xcb-randr
    xcb-keysyms
    xcb-ewmh
    X11
)