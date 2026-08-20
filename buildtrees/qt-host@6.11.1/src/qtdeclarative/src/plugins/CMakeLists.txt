# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_qml_debug AND QT_FEATURE_thread)
    add_subdirectory(qmltooling)
endif()
if(TARGET Qt::Quick)
    add_subdirectory(scenegraph)
endif()

if((NOT CMAKE_CROSSCOMPILING) OR QT_FORCE_BUILD_TOOLS)
    add_subdirectory(qmllint)
endif()

if(TARGET Qt::QmlLSPrivate)
    add_subdirectory(qmlls)
endif()
