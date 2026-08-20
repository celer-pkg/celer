# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Widgets)
    add_subdirectory(sensor_explorer_widgets)
endif()
if (TARGET Qt::Quick)
    add_subdirectory(sensorclerk)
    add_subdirectory(sensor_explorer_qml)
endif()
