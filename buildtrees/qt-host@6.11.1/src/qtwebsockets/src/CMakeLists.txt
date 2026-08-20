# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(websockets)
if(QT_FEATURE_websockets_qml)
    add_subdirectory(imports)
endif()
