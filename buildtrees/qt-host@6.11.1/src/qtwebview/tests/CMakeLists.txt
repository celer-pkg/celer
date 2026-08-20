# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_BUILD_STANDALONE_TESTS)
    find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS WebEngineCore WebEngineQuick)
endif()

qt_build_tests()
