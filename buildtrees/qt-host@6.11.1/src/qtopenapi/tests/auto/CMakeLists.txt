# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Check for both existence of the built Qt generator plugin and the
# upstream generator.
# It's possible to have java + maven available, so the qt plugin will be built,
# but for the upstream generator to be missing, in which case we can't build the tests.
if(TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar
        AND TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator
        AND TARGET Qt::OpenApiCommon
    )
    add_subdirectory(compression)
    add_subdirectory(generator-additional-properties)
    add_subdirectory(linking_several_clients)
    if(TARGET Qt::Gui)
        add_subdirectory(mediatype)
    endif()
    add_subdirectory(models)
    add_subdirectory(openapi2.0)
    add_subdirectory(operation-parameters)
    add_subdirectory(petstore)
    add_subdirectory(responses)
    add_subdirectory(cmake)
    add_subdirectory(server_configuration)
else()
    set(missing_text "")
    if(NOT TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar)
        set(missing_text "Qt OpenAPI generator plugin target is missing.")
    endif()
    if(NOT TARGET WrapOpenAPIGenerator::WrapOpenAPIGenerator)
        set(missing_text "Upstream OpenAPI generator target is missing.")
    endif()
    if(NOT TARGET Qt::OpenApiCommon)
        set(missing_text "Qt OpenAPI Common library target is missing.")
    endif()
    # Only show when building standalone tests.
    # Otherwise we already say when skip the tests when configuring
    # them in-tree with the repo in the configure summary section.
    if(NOT QT_SUPERBUILD AND QT_BUILD_STANDALONE_TESTS)
        message(STATUS "Skipping OpenAPI tests because ${missing_text}")
    endif()
endif()
