# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This is just a placeholder module that in the future might ship public headers.
qt_internal_add_module(OpenApi
    HEADER_MODULE
    NO_MODULE_HEADERS
    NO_PRIVATE_MODULE
    NO_GENERATE_CPP_EXPORTS
    NO_ADDITIONAL_TARGET_INFO
    NO_GENERATE_METATYPES
    NO_PACKAGE_CONFIG_FILE
    NO_MODULE_JSON_FILE
    NO_QMAKE_SUPPORT_FILES
)
set_target_properties(OpenApi PROPERTIES AUTOMOC OFF)

# Make find_package(Qt6 COMPONENTS OpenApi) lookup the OpenApiCommon library as a dependency as
# well.
if(TARGET Qt::OpenApiCommon)
    qt_internal_register_target_dependencies(OpenApi PUBLIC Qt::OpenApiCommon)
endif()

qt_internal_add_docs(OpenApi
    doc/qtopenapi.qdocconf
)

# Add the standalone tool package that exposes the Qt openapi generator plugin.
set(base_name "${QT_CMAKE_EXPORT_NAMESPACE}OpenApiTools")
set(base_path "${CMAKE_CURRENT_SOURCE_DIR}/${base_name}")

qt_internal_add_tools_package(
    PACKAGE_BASE_NAME OpenApi
    EXTRA_CMAKE_FILES
        "${base_path}ExtraTargets.cmake"
        "${base_path}GeneratorMacros.cmake"
        "${base_path}CopyHeadersScript.cmake"
    EXTRA_CMAKE_INCLUDES
        "${base_name}ExtraTargets.cmake"
        "${base_name}GeneratorMacros.cmake"
)

# Make find_package(Qt6 COMPONENTS OpenApiTools) also find the java runtime and the upstream
# generator.
qt_internal_record_tools_package_extra_third_party_dependency(
    PACKAGE_BASE_NAME OpenApi
    DEPENDENCY_PACKAGE_NAME WrapOpenAPIJava)
qt_internal_record_tools_package_extra_third_party_dependency(
    PACKAGE_BASE_NAME OpenApi
    DEPENDENCY_PACKAGE_NAME WrapOpenAPIGenerator)

# Make find_package(Qt6 COMPONENTS OpenApi) also find the OpenApiTools package.
qt_record_extra_qt_main_tools_package_dependency(OpenApi OpenApiTools "${PROJECT_VERSION}")

