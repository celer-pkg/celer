# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from client.pro.

add_subdirectory(shared)

# webOS has a modified version of QtWayland and does not support e.g. multiple window creation
# in a single client, attempting to do so will cause a segmentation fault
if (NOT WEBOS)
    add_subdirectory(iviapplication)
endif()
