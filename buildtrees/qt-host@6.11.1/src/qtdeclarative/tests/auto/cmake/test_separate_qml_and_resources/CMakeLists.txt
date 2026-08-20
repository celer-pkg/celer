cmake_minimum_required(VERSION 3.16)
project(test_separate_qml_and_resources LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Test)
qt_standard_project_setup(REQUIRES 6.10)

qt_add_qml_module(MyQmlModule
    STATIC
    URI org.qt-project.test_separate_qml_and_resources
    VERSION 1.0
)

qt_target_qml_sources(MyQmlModule
    QML_FILES
        Main.qml
)

qt_target_qml_sources(MyQmlModule
    RESOURCES
        file.txt
)

add_executable(app
    main.cpp
)

target_link_libraries(app PRIVATE MyQmlModule MyQmlModuleplugin)
