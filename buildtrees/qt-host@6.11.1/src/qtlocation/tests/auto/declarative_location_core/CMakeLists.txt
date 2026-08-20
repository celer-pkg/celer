file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.qml)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_declarative_location_core
    QMLTEST
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Location
        Qt::LocationPrivate
        Qt::Quick
    TESTDATA ${test_data}
)
