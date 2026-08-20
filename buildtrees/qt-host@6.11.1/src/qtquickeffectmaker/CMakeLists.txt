# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
include(FeatureSummary)

include(.cmake.conf)
project(QQEffectMaker
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Quick Effect Maker"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui Quick ShaderTools)

# Quick3DGlslParserPrivate is optional
find_package(Qt6 ${PROJECT_VERSION} OPTIONAL_COMPONENTS Quick3DGlslParserPrivate)
set_package_properties(Qt6Quick3DGlslParserPrivate PROPERTIES
    TYPE OPTIONAL
    PURPOSE "- Enables Syntax Highlighting"
)

if(NOT TARGET Qt::Quick)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Quick\" is not met.")
    return()
endif()

if(NOT TARGET Qt::ShaderTools)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::ShaderTools\" is not met.")
    return()
endif()

if(INTEGRITY OR QNX OR VXWORKS OR WATCHOS)
    message(NOTICE "Skipping the build as the condition \"NOT INTEGRITY AND NOT QNX AND NOT VXWORKS AND NOT WATCHOS\" is not met.")
    return()
endif()

# Note: Temporarily build only for Windows and macOS
#if(NOT WIN32 AND NOT MACOS)
#    message(NOTICE "Skipping the build as the condition \"WIN32 OR MACOS\" is not met.")
#    return()
#endif()

if(WASM)
    message(NOTICE "Skipping the build as the condition \"NOT WASM\" is not met.")
    return()
endif()

if(CMAKE_CROSSCOMPILING)
    message(NOTICE "Skipping the build as the condition \"NOT CMAKE_CROSSCOMPILING\" is not met.")
    return()
endif()

# Copy nodes content
set(nodes_folders
    "nodes/basic"
    "nodes/common"
    "nodes/extra"
    "nodes/images"
    "nodes/nature"
)

qt_build_repo()

qt_path_join(nodes_path "${QT_INSTALL_DIR}" "${INSTALL_QMLDIR}/QtQuickEffectMaker/defaultnodes")
message(NOTICE "Nodes path: ${nodes_path}")
qt_copy_or_install(DIRECTORY ${nodes_folders} DESTINATION ${nodes_path})
