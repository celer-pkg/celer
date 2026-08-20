# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_baseline_mapitems
    SOURCES
        ../../../../qtbase/tests/baseline/shared/baselineprotocol.cpp
        ../../../../qtbase/tests/baseline/shared/baselineprotocol.h
        ../../../../qtbase/tests/baseline/shared/lookup3.cpp
        ../../../../qtbase/tests/baseline/shared/qbaselinetest.cpp
        ../../../../qtbase/tests/baseline/shared/qbaselinetest.h
        tst_baseline_mapitems.cpp
    INCLUDE_DIRECTORIES
        ../../../../qtbase/tests/baseline/shared
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::LocationPrivate
        Qt::Positioning
        Qt::Qml
        Qt::Quick
    TESTDATA ${test_data}
)

# Make the QML files available to Creator's locator.
target_sources(tst_baseline_mapitems
    PRIVATE
        ${test_data}
)

#add_dependencies(tst_baseline_mapitems qmlscenegrabber)

set_source_files_properties(${test_data}
    PROPERTIES
        HEADER_FILE_ONLY ON
)
