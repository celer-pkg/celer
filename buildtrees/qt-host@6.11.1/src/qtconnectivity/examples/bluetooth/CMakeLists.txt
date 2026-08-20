# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(heartrate-server)
if(TARGET Qt6::Widgets)
    qt_internal_add_example(btchat)
endif()
if(TARGET Qt6::Quick)
    qt_internal_add_example(lowenergyscanner)
    qt_internal_add_example(heartrate-game)
endif()
