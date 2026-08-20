# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# For now, don't built auto tests when QT_BUILD_MINIMAL_STATIC_TEST
# is specified and the build is targeting iOS. QT_BUILD_MINIMAL_STATIC_TEST is used in our CI.
# Regular non-cmake build tests shouldn't be built because the CI will try to run them and fail
# due to missing simulator support.
if(IOS AND QT_BUILD_MINIMAL_STATIC_TESTS)
    return()
endif()

add_subdirectory(tga)
add_subdirectory(wbmp)
add_subdirectory(dds)
add_subdirectory(icns)
add_subdirectory(jp2)
add_subdirectory(webp)
add_subdirectory(heif)
add_subdirectory(mng)
add_subdirectory(tiff)
