# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Evaluate features to decide what to build.
# The config files will be written in the src/global module.
qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/../configure.cmake")

add_subdirectory(global)
if(QT_FEATURE_scxml)
    add_subdirectory(scxml)
    if(QT_FEATURE_scxml_qml)
        add_subdirectory(scxmlqml)
    endif()
endif()
if(QT_FEATURE_statemachine)
    add_subdirectory(statemachine)
    if(QT_FEATURE_statemachine_qml)
        add_subdirectory(statemachineqml)
    endif()
endif()
add_subdirectory(plugins)
