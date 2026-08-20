# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgsurface_series
    SOURCES
        tst_series.cpp
    LIBRARIES
        Qt::Gui
        Qt::Graphs
)

set(qgsurface_series_resource_files
    "customtexture.jpg"
)

qt_internal_add_resource(tst_qgsurface_series "qgdsurface_series"
    PREFIX
        "/"
    FILES
        ${qgsurface_series_resource_files}
)
