# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_buildtimeqsb LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_buildtimeqsb
    SOURCES
        tst_buildtimeqsb.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::ShaderToolsPrivate
)

qt6_add_shaders(tst_buildtimeqsb "shaders"
    PREFIX
        "/test"
    FILES
        "color.vert"
        "color.frag"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_precompile"
    PRECOMPILE
    PREFIX
        "/test"
    FILES
        "color.vert"
        "color.frag"
    OUTPUTS
        "color_precomp.vert.qsb"
        "color_precomp.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_in_subdir"
    PREFIX
        "/some/prefix"
    FILES
        "subdir/texture.vert"
        "subdir/texture.frag"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_in_subdir_with_outputs_as_alias"
    PREFIX
        "/some/prefix"
    FILES
        "subdir/texture.vert"
        "subdir/texture.frag"
   OUTPUTS
       "alias_texture.vert.qsb"
       "x/y/z/alias_texture.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_in_subdir_with_base"
    BATCHABLE
    PREFIX
        "/base_test"
    BASE
        "subdir"
    FILES
        "subdir/texture2.vert"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_custom_targets_and_names"
    PREFIX
        "/test"
    GLSL
        "300es,330"
    NOHLSL
    NOMSL
    FILES
        "color.vert"
        "color.frag"
    OUTPUTS
        "color_1.vert.qsb"
        "color_1.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_custom_targets_and_names_batchable"
    PREFIX
        "/test"
    BATCHABLE
    GLSL
        "300es,330"
    NOHLSL
    NOMSL
    FILES
        "color.vert"
        "color.frag"
    OUTPUTS
        "color_1b.vert.qsb"
        "color_1b.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_with_defines"
    PREFIX
        "/"
    DEFINES
        "MY_DEFINE"
        "ANOTHER_DEFINE"
    FILES
        "subdir/test/texture_def.frag"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_with_replacements"
    PREFIX
        "/test"
    FILES
        "color.vert@glsl,100es,replacements/r1.vert@spirv,100,replacements/r2.spv@hlsl,50,replacements/r3.vert"
        "color.frag@msl,12,replacements/r4.frag"
    OUTPUTS
        "color_repl.vert.qsb"
        "x/color_repl.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_tessellation_part1"
    PREFIX
        "/test"
    GLSL
        "410,320es"
    TESSELLATION
    FILES
        "tess.vert"
        "tess.frag"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_tessellation_part2"
    PREFIX
        "/test"
    NOHLSL
    GLSL
        "410,320es"
    TESSELLATION
    TESSELLATION_VERTEX_COUNT
        3
    TESSELLATION_MODE
        "triangles"
    FILES
        "tess.tesc@hlsl,50,tess_hull.hlsl"
        "tess.tese@hlsl,50,tess_domain.hlsl"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_multiview"
    PREFIX
        "/test"
    GLSL
        "330,300es"
    HLSL
        61
    MSL
        12
    VIEW_COUNT
        2
    FILES
        "color_multiview.vert"
        "color.frag"
    OUTPUTS
        "color_multiview.vert.qsb"
        "color_multiview.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_multiview_auto"
    PREFIX
        "/test"
    MULTIVIEW
    FILES
        "color_multiview_def.vert"
        "color.frag"
    OUTPUTS
        "color_multiview_def.vert.qsb"
        "color_multiview_def.frag.qsb"
)

qt6_add_shaders(tst_buildtimeqsb "shaders_with_original_filename"
    PRECOMPILE
    PREFIX
        "/test"
    FILES
        "color.vert"
        "color.frag"
    OUTPUTS
        "color_99.vert.qsb"
        "color_99.frag.qsb"
    ORIGINAL_FILES
        # evil
        "subdir/texture.vert"
        "subdir/texture.frag"
)
