# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT INTEGRITY AND NOT rtems)
    add_subdirectory(assetimporters)
endif()
if (QT_FEATURE_qml_debug AND TARGET Qt::PacketProtocolPrivate)
    add_subdirectory(qmldbg_quick3dprofiler)
endif()
