# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


add_subdirectory(enums)
add_subdirectory(pods)
if(NOT windows)
    add_subdirectory(signature)
endif()
