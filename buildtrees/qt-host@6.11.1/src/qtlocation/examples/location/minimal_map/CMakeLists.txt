# Generated from minimal_map.pro.

cmake_minimum_required(VERSION 3.16)
project(minimal_map LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/minimal_map")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Location)
find_package(Qt6 COMPONENTS Qml)

qt_add_executable(minimal_map WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(minimal_map PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Qml
)


# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(minimal_map "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS minimal_map
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
