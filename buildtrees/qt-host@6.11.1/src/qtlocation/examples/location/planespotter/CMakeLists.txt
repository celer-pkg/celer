# Generated from planespotter.pro.

cmake_minimum_required(VERSION 3.16)
project(planespotter LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/planespotter")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)

qt_add_executable(planespotter WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(planespotter PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Positioning
    Qt::Qml
    Qt::Quick
)


# Resources:
set(qml_resource_files
    "Plane.qml"
    "airplane.svg"
    "planespotter.qml"
)

qt6_add_resources(planespotter "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS planespotter
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
