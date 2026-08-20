file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.qml)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_declarative_mappolyline
    QMLTEST
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Location
        Qt::Quick
    TESTDATA ${test_data}
)
