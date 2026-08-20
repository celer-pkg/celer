# Generated from places_map.pro.

cmake_minimum_required(VERSION 3.16)
project(places_map LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/places_map")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)

qt_add_executable(places_map WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(places_map PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Positioning
    Qt::Qml
    Qt::Quick
)


# Resources:
set(places_map_resource_files
    "marker.png"
    "Main.qml"
    "PermissionsScreen.qml"
    "PlacesMap.qml"
)

qt6_add_resources(places_map "places_map"
    PREFIX
        "/"
    FILES
        ${places_map_resource_files}
)

install(TARGETS places_map
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
