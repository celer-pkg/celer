# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Widgets)
    add_subdirectory(draganddrop)
    add_subdirectory(embedded)
    add_subdirectory(svggenerator)
    add_subdirectory(svgwidget)
    add_subdirectory(embeddedsvgviewer)
    add_subdirectory(textobject)
endif()
