# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(android_app OR
    (QT_FEATURE_commandlineparser
        AND NOT ANDROID
        AND NOT UIKIT
        AND NOT WASM
        AND NOT rtems))
        add_subdirectory(canbusutil)
endif()
