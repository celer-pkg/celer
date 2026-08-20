cmake_minimum_required(VERSION 3.16)

project(FXMaterialShowroom LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Gui Qml Quick Quick3D)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(FxMaterialShowroomApp
    src/app_environment.h
    src/main.cpp
    src/imagedownloader.cpp
    src/imagedownloader.h
)

qt_add_resources(FxMaterialShowroomApp "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(FxMaterialShowroomApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

include(qmlmodules)

install(TARGETS FxMaterialShowroomApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET FxMaterialShowroomApp
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
