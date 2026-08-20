# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# We need to evaluate the features early, to decide whether to build and install the
# OpenAPI module at all. Otherwise if not all dependencies are met and the generator is not built
# in the tools subdir, we still end up installing the macros and extra targets files, which is
# undesirable.
add_subdirectory(global)

# First build the generator tool, before the common library is created.
add_subdirectory(tools)

if(QT_FEATURE_openapi_generator)
    if(QT_FEATURE_openapi_common_library)
        add_subdirectory(openapicommon)
    endif()

    # We create the openapi module last, because we want to add a dependency on the common library
    # on the CMake package level.
    add_subdirectory(openapi)
else()
    # Enable documentation build only
    add_subdirectory(openapi/doc)
endif()
