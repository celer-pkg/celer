# Generated from places_list.pro.

cmake_minimum_required(VERSION 3.16)
project(places_list LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/places_list")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)

qt_add_executable(places_list WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(places_list PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Positioning
    Qt::Qml
    Qt::Quick
)


# Resources:
set(places_list_resource_files
    "Marker.qml"
    "marker.png"
    "places_list.qml"
)

qt6_add_resources(places_list "places_list"
    PREFIX
        "/"
    FILES
        ${places_list_resource_files}
)

install(TARGETS places_list
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
