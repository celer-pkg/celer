# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## testcon Binary:
#####################################################################

qt_internal_add_app(testcon
    SOURCES
        ambientproperties.cpp ambientproperties.h ambientproperties.ui
        changeproperties.cpp changeproperties.h changeproperties.ui
        controlinfo.cpp controlinfo.h controlinfo.ui
        docuwindow.cpp docuwindow.h
        invokemethod.cpp invokemethod.h invokemethod.ui
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        sandboxing.cpp sandboxing.h
    LIBRARIES
        Qt::AxContainer
        Qt::AxContainerPrivate
        Qt::AxServer
        Qt::CorePrivate
        Qt::Gui
        Qt::PrintSupport
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

# Resources:
set(testcon_resource_files
    "images/controlmethods.png"
    "images/controlproperties.png"
    "images/filenew.png"
)

qt_internal_add_resource(testcon "testcon"
    PREFIX
        "/"
    FILES
        ${testcon_resource_files}
)
