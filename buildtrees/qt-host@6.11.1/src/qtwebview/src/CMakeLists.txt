# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(webview)
add_subdirectory(quick)
add_subdirectory(plugins)
if(ANDROID)
    add_subdirectory(jar)
endif()
