# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(iconengines)
if(QT_FEATURE_imageformatplugin)
    add_subdirectory(imageformats)
endif()
