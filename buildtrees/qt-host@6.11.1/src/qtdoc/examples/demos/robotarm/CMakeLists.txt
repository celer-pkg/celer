cmake_minimum_required(VERSION 3.16)

project(RobotArm LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick Quick3D)
qt_standard_project_setup(REQUIRES 6.8)
qt_add_executable(RobotArmApp src/main.cpp)

qt_add_resources(RobotArmApp "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(RobotArmApp PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
)

add_subdirectory(Backend)

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

install(TARGETS RobotArmApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET RobotArmApp
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
