# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_gui
   AND NOT INTEGRITY
   AND TARGET Qt6::Concurrent
   AND TARGET Qt6::Network
   AND TARGET Qt6::Qml
   AND TARGET Qt6::TaskTree)
    add_subdirectory(assetdownloader)
endif()
