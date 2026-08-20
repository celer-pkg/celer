# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(add_qt_statecharts target)
    # Don't try to add statecharts when cross compiling, and the target is actually a host target
    # (like a tool).
    qt_is_imported_target("${target}" is_imported)
    if(is_imported)
        return()
    endif()

    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "FILES")
    _qt_internal_validate_all_args_are_parsed(arg)

    qt6_add_statecharts(${target} ${arg_FILES})
endfunction()
