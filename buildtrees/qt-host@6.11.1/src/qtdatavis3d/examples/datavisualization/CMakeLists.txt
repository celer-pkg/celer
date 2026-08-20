# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Quick)
    qt_internal_add_example(qmlaxishandling)
    qt_internal_add_example(qmlbars)
    qt_internal_add_example(qmlscatter)
    qt_internal_add_example(qmlsurfacegallery)
endif()
if(NOT ANDROID AND NOT IOS AND NOT WINRT)
    qt_internal_add_example(graphgallery)
    qt_internal_add_example(volumetric)
endif()
