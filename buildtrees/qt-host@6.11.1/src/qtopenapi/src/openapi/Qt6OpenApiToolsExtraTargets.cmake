# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT TARGET "${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar")
    add_executable("${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar" IMPORTED GLOBAL)

    set(_qt_openapi_generator_path
        "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_LIBEXECS}/cpp-qt6-client-openapi-generator.jar")

    set_target_properties("${QT_CMAKE_EXPORT_NAMESPACE}::QtOpenAPIGeneratorJar"
        PROPERTIES IMPORTED_LOCATION "${_qt_openapi_generator_path}")
endif()
