# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::Widgets AND TARGET Qt::SerialBus)
    add_subdirectory(modbus)
endif()
