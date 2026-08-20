# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QJp2Plugin Plugin:
#####################################################################

qt_internal_add_plugin(QJp2Plugin
    OUTPUT_NAME qjp2
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qjp2handler.cpp qjp2handler_p.h
    LIBRARIES
        WrapJasper::WrapJasper
        Qt::Core
        Qt::Gui
)
