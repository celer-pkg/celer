find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Test)

# Opt-into 6.9 behaviors to avoid QTP0004 warnings about needing
# qmldir files in each extra subdirectory, which fails the test
# because they go to stderr.
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(main)

qt_add_qml_module(main
    URI MyAppUri
    QML_FILES
        Main.qml
)

target_link_libraries(main
    PRIVATE
        Qt::Quick
        Qt::Test
)

add_subdirectory(../src src)
