# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## axviewer Binary:
#####################################################################

qt_internal_add_manual_test(tst_axviewer_manual
    SOURCES
        ../shared/metaobjectdump.cpp ../shared/metaobjectdump.h
        ../shared/textdialog.cpp ../shared/textdialog.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::AxContainer
        Qt::Gui
        Qt::Widgets
)

## Scopes:
#####################################################################

set(diaglib_path "${CMAKE_CURRENT_SOURCE_DIR}/../../../../qtbase/tests/manual/diaglib")
qt_internal_extend_target(tst_axviewer_manual CONDITION EXISTS "${diaglib_path}"
    SOURCES
        "${diaglib_path}/eventfilter.cpp" "${diaglib_path}/eventfilter.h"
        "${diaglib_path}/nativewindowdump.h"
        "${diaglib_path}/qwindowdump.cpp" "${diaglib_path}/qwindowdump.h"
        "${diaglib_path}/textdump.cpp" "${diaglib_path}/textdump.h"
    DEFINES
        QT_DIAG_LIB
    INCLUDE_DIRECTORIES
        "${diaglib_path}"
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
)

qt_internal_extend_target(tst_axviewer_manual CONDITION (EXISTS "${diaglib_path}") AND (WIN32)
    SOURCES
        "${diaglib_path}/nativewindowdump_win.cpp"
    LIBRARIES
        user32
)

qt_internal_extend_target(tst_axviewer_manual CONDITION (EXISTS "${diaglib_path}") AND (NOT (WIN32))
    SOURCES
        "${diaglib_path}/nativewindowdump.cpp"
)

qt_internal_extend_target(tst_axviewer_manual CONDITION (EXISTS "${diaglib_path}") AND
    TARGET Qt::Widgets
    SOURCES
        "${diaglib_path}/debugproxystyle.cpp" "${diaglib_path}/debugproxystyle.h"
        "${diaglib_path}/logwidget.cpp" "${diaglib_path}/logwidget.h"
        "${diaglib_path}/qwidgetdump.cpp" "${diaglib_path}/qwidgetdump.h"
    LIBRARIES
        Qt::WidgetsPrivate
)

qt_internal_extend_target(tst_axviewer_manual CONDITION (EXISTS "${diaglib_path}") AND
    TARGET Qt::OpenGL AND TARGET Qt::OpenGLWidgets
    SOURCES
        "${diaglib_path}/glinfo.cpp" "${diaglib_path}/glinfo.h"
    LIBRARIES
        Qt::OpenGL
        Qt::OpenGLWidgets
)
