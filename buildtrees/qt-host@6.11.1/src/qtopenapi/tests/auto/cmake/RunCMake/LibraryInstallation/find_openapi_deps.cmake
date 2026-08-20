# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(deps_available TRUE)

# Try to find the openapi deps. If they are missing, we will skip the test, because
# not all CI platforms have them installed.
find_package(Qt6 COMPONENTS OpenApi OpenApiCommon)

if(NOT TARGET Qt6::OpenApi OR NOT TARGET Qt6::OpenApiCommon)
    set(deps_available FALSE)
endif()

file(WRITE "${DEPS_AVAILABLE_INCLUDE_PATH}" "set(DEPS_AVAILABLE \"${deps_available}\")\n")
