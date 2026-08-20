# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin
add_subdirectory(3rdparty/poly2tri)
add_subdirectory(3rdparty/clipper)
add_subdirectory(3rdparty/clip2tri)
add_subdirectory(positioning)
add_subdirectory(plugins)
if(TARGET Qt::Quick)
    add_subdirectory(positioningquick)
endif()
# special case end
