# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#.rst:
# FindBundletool
# ---------
#
# Try to locate the android bundletool.
# If found, this will define the following variables:
#
# ``Bundletool_FOUND``
#     True if the Bundletool is found
#
# ``Bundletool_EXECUTABLE ``
#     Path to the Bundletool executable
#
# If ``Bundletool_FOUND`` is TRUE, it will also define the following
# imported target:
#
# ``Bundletool::Bundletool``
#     The Bundletool executable

if(DEFINED ENV{Bundletool_EXECUTABLE})
    if((NOT Bundletool_EXECUTABLE OR NOT EXISTS "${Bundletool_EXECUTABLE}")
        AND EXISTS "$ENV{Bundletool_EXECUTABLE}")
        set(_Bundletool_use_force FORCE)
    else()
        set(_Bundletool_use_force "")
    endif()
        set(Bundletool_EXECUTABLE "$ENV{Bundletool_EXECUTABLE}" CACHE FILEPATH
            "Path to the 'bundletool' executable." ${_Bundletool_use_force})
    unset(_Bundletool_use_force)
endif()

find_file(Bundletool_EXECUTABLE bundletool bundletool.jar)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Bundletool DEFAULT_MSG Bundletool_EXECUTABLE)

if(Bundletool_FOUND)
    if(NOT TARGET Bundletool::Bundletool)
        add_executable(Bundletool::Bundletool IMPORTED)
        set_target_properties(Bundletool::Bundletool PROPERTIES
            IMPORTED_LOCATION "${Bundletool_EXECUTABLE}")
    endif()
endif()

mark_as_advanced(Bundletool_EXECUTABLE)
