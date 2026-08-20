# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qstate Test:
#####################################################################

qt_internal_add_test(tst_qstate
    SOURCES
        tst_qstate.cpp
    LIBRARIES
        Qt::StateMachine
)
