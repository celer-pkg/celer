# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

add_subdirectory(imageviewer)
add_subdirectory(jsonviewer)
add_subdirectory(txtviewer)

if(TARGET Qt6::PdfWidgets)
    add_subdirectory(pdfviewer)
endif()

if(TARGET Qt6::Quick3D)
    add_subdirectory(Q3DViewer)
endif()
