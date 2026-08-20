# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## testcontrol Binary:
#####################################################################

qt_internal_add_manual_test(tst_testcontrol_manual
    GUI
    SOURCES
        main.cpp
        testcontrol.rc
    LIBRARIES
        Qt::AxServer
        Qt::Gui
        Qt::Widgets
)
qt_disable_warnings(tst_testcontrol_manual)
qt6_target_idl(tst_testcontrol_manual)
