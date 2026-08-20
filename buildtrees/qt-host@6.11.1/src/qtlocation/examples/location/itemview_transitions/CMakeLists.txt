# Generated from itemview_transitions.pro.

cmake_minimum_required(VERSION 3.16)
project(itemview_transitions LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/itemview_transitions")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Location)

qt_add_executable(itemview_transitions WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(itemview_transitions PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Qml
    Qt::Quick
)


file(GLOB resource_glob_0 RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.qml")
foreach(file IN LISTS resource_glob_0)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/${file}" PROPERTIES QT_RESOURCE_ALIAS "${file}")
endforeach()

# Resources:
set(qmake_qmlfiles_resource_files
    "${resource_glob_0}"
)

qt6_add_resources(itemview_transitions "qmake_qmlfiles"
    PREFIX
        "/"
    BASE
        "$$PWD"
    FILES
        ${qmake_qmlfiles_resource_files}
)

install(TARGETS itemview_transitions
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
