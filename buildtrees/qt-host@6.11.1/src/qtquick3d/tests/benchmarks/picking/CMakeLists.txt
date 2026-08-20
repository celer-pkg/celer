# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(picking)
if(QT_FEATURE_private_tests)
    add_subdirectory(intersection)
    add_subdirectory(bvh)
endif()
