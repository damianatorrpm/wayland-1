cmake_minimum_required(VERSION 3.10.0)

project("Wayland"
    VERSION "0.0.0"
    DESCRIPTION "Wayland client and server extensions"
    LANGUAGES CXX C
)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "1.0.0" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Set minimum versions required:
set(QT_MIN_VERSION "5.12.0")

## Liri specific setup common for all modules:
include(LiriSetup)

## Find Qt 5:
find_package(Qt5 "${QT_MIN_VERSION}"
    CONFIG REQUIRED
    COMPONENTS
        Core
        Qml
        Quick
        Gui
        WaylandCompositor
        WaylandClient
)

## Add subdirectories:
add_subdirectory(src/waylandclient)
add_subdirectory(src/waylandserver)
add_subdirectory(src/imports/waylandclient)
add_subdirectory(src/imports/waylandserver)
