# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgsurface_heightproxy
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::Gui
        Qt::Graphs
)

set(qgsurface_heightproxy_resource_files
    "customtexture.jpg"
)

qt_internal_add_resource(tst_qgsurface_heightproxy "qgdsurface_heightproxy"
    PREFIX
        "/"
    FILES
        ${qgsurface_heightproxy_resource_files}
)
