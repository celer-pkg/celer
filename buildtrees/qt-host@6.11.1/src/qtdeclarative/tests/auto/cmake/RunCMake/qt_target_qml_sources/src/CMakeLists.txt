target_sources(main PRIVATE
    main.cpp
)

if(RunCMake_TEST STREQUAL "subdirectory_only")
    qt_target_qml_sources(main
        QML_FILES
            Main.qml
            SubPage.qml
    )
elseif(RunCMake_TEST STREQUAL "subdirectory_with_top"
        OR RunCMake_TEST STREQUAL "subdirectory_without_qt_in_root")
    qt_target_qml_sources(main
        QML_FILES
            SubPage.qml
    )
else()
    message(FATAL_ERROR "Unknown test-case: ${RunCMake_TEST}")
endif()

