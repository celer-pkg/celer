# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Widgets)
    add_subdirectory(desktopservices)
endif()
if(QT_FEATURE_process AND TARGET Qt::Widgets)
    add_subdirectory(fluidlauncher)
endif()
