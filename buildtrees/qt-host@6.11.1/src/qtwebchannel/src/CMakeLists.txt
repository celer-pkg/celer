# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from src.pro.

add_subdirectory(webchannel)
if(TARGET Qt::Quick)
    add_subdirectory(webchannelquick)
endif()
