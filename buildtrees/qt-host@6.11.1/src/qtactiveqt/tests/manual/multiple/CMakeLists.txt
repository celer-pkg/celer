# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## multiple Binary:
#####################################################################

set(CMAKE_AUTOMOC ON)

qt_internal_add_manual_test(tst_multiple
    GUI
    SOURCES
        ax1.h
        ax2.h
        main.cpp
        multipleax.def
        multipleax.rc
    LIBRARIES
        Qt::AxServer
        Qt::Gui
        Qt::Widgets

)
# qt_disable_warnings(tst_multiple)
qt6_target_idl(tst_multiple)
