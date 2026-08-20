# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT WASM AND NOT IOS AND NOT ANDROID AND NOT QNX AND NOT INTEGRITY AND NOT WEBOS)
    add_subdirectory(cooker)
endif()
