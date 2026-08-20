# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(q3dsurface-heightproxy_datavis
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::Gui
        Qt::DataVisualization
)

set(q3dsurface-heightproxy_resource_files
    "customtexture.jpg"
)

qt_internal_add_resource(q3dsurface-heightproxy_datavis "q3dsurface-heightproxy"
    PREFIX
        "/"
    FILES
        ${q3dsurface-heightproxy_resource_files}
)
