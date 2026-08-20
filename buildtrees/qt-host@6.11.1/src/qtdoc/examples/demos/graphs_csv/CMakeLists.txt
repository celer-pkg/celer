cmake_minimum_required(VERSION 3.16)

project(qtgraphscsv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rdparty)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Graphs)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appqtgraphscsv
    main.cpp
)

set_source_files_properties(Units.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appqtgraphscsv
    URI qtgraphscsv
    QML_FILES
        Main.qml
        Units.qml
        components/CustomTableView.qml
        components/Graph.qml
        components/LegendItem.qml
        components/HorizontalHeaderDelegate.qml
        components/VerticalHeaderDelegate.qml
    SOURCES
        datamodel.cpp datamodel.h
)

qt_add_resources(appqtgraphscsv "data"
    PREFIX
        "/data"
    BASE
        "data"
    FILES
        "data/medals.csv"
)

set_target_properties(appqtgraphscsv PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.demos.appqtgraphscsv
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqtgraphscsv
    PRIVATE
        Qt6::Quick
        Qt6::Graphs
        csv
)

include(GNUInstallDirs)
install(TARGETS appqtgraphscsv
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
