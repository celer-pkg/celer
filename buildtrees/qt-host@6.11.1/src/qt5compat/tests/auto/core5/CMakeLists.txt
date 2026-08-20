# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from core5.pro.

add_subdirectory(codecs)
add_subdirectory(io)
if (TARGET Qt::Xml)
    add_subdirectory(sax)
endif()
add_subdirectory(serialization)
add_subdirectory(text)
add_subdirectory(tools)
