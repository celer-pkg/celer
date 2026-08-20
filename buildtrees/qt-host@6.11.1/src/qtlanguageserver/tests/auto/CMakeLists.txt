# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(cmake)

# For now, don't built auto tests when QT_BUILD_MINIMAL_STATIC_TEST
# is specified and the build is targeting iOS. QT_BUILD_MINIMAL_STATIC_TEST is used in our CI.
# Regular non-cmake build tests shouldn't be built because the CI will try to run them and fail
# due to missing simulator support.
if(IOS AND QT_BUILD_MINIMAL_STATIC_TESTS)
    return()
endif()

add_subdirectory(qjsonrpcprotocol)
add_subdirectory(typedjson)
add_subdirectory(qlanguageserver)
add_subdirectory(generated_lsp_types)
