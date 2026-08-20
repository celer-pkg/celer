qt_add_library(with_subdir_test_module STATIC)
qt_autogen_tools_initial_setup(with_subdir_test_module)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(with_subdir_test_module
    URI WithSubDir
    QML_FILES
        qml/GreenRect.qml
        qml/MyScript.js

    # Hide it at compile time. Otherwise the "TestTypes" module may see it,
    # but the "verify" module won't.
    OUTPUT_DIRECTORY HiddenWithSubdir

    # Don't complain about mismatched target paths
    NO_LINT
    __QT_INTERNAL_NO_GENERATE_QMLLS_BUILD_INI
)

target_link_libraries(with_subdir_test_module
    PRIVATE Qt6::Qml
)

qt_autogen_tools_initial_setup(with_subdir_test_moduleplugin)
