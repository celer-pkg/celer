# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt6::qtprotobufgen AND TARGET Qt6::qtgrpcgen)
    qt_internal_add_example(clientguide)
    if(TARGET Qt6::ProtobufQuick AND TARGET Qt6::QuickControls2)
        qt_internal_add_example(magic8ball)

        if(TARGET Qt6::ProtobufWellKnownTypes)
            qt_internal_add_example(vehicle)
        endif()

        if(QT_FEATURE_clipboard AND QT_FEATURE_quickcontrols2_material
                AND TARGET Qt6::ProtobufQtCoreTypes AND TARGET Qt6::GrpcQuick)
            qt_internal_add_example(chat)
        endif()
    endif()
endif()

