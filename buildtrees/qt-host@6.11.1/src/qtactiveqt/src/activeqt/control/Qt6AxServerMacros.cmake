# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Adds an ActiveX server executable, generates an IDL file and links the produced .tbl to the
# executable.
# Arguments: See qt6_target_idl
#
# This function is currently in Technology Preview.
# Its signature and behavior might change.
function(qt6_add_axserver_executable target)
    cmake_parse_arguments(arg "NO_AX_SERVER_REGISTRATION" "" "" ${ARGN})
    if(arg_NO_AX_SERVER_REGISTRATION)
        set(arg_NO_AX_SERVER_REGISTRATION "NO_AX_SERVER_REGISTRATION")
    else()
        unset(arg_NO_AX_SERVER_REGISTRATION)
    endif()
    qt_add_executable(${target} ${arg_UNPARSED_ARGUMENTS})
    set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(${target} PRIVATE ${QT_CMAKE_EXPORT_NAMESPACE}::AxServer)
    qt6_target_idl(${target} ${arg_NO_AX_SERVER_REGISTRATION})
endfunction()

# Adds an ActiveX server library, generates an IDL file and links the produced .tbl to the
# dll.
# Arguments: See qt6_target_idl
#
# This function is currently in Technology Preview.
# Its signature and behavior might change.
function(qt6_add_axserver_library target)
    cmake_parse_arguments(arg "NO_AX_SERVER_REGISTRATION" "" "" ${ARGN})
    if(arg_NO_AX_SERVER_REGISTRATION)
        set(arg_NO_AX_SERVER_REGISTRATION "NO_AX_SERVER_REGISTRATION")
    else()
        unset(arg_NO_AX_SERVER_REGISTRATION)
    endif()
    add_library(${target} SHARED ${arg_UNPARSED_ARGUMENTS})
    target_link_libraries(${target} PRIVATE ${QT_CMAKE_EXPORT_NAMESPACE}::AxServer)
    qt6_target_idl(${target} ${arg_NO_AX_SERVER_REGISTRATION})
endfunction()

# Adds post-build rules to generate and link IDC/MIDL artifacts to the library or executable.
# Arguments:
#   NO_AX_SERVER_REGISTRATION skips the ActiveX server registration.
#      Note: You may also use the QT_NO_AX_SERVER_REGISTRATION variable to globally skip
#      the ActiveX server registrations.
#
# This function is currently in Technology Preview.
# Its signature and behavior might change.
function(qt6_target_idl target)
    cmake_parse_arguments(arg "NO_AX_SERVER_REGISTRATION" "" "" ${ARGN})
    if(NOT WIN32)
        return()
    endif()

    set(output_idl "${CMAKE_CURRENT_BINARY_DIR}/${target}$<CONFIG>.idl")
    set(output_tlb "${CMAKE_CURRENT_BINARY_DIR}/${target}$<CONFIG>.tlb")

    _qt_internal_get_tool_wrapper_script_path(tool_wrapper)
    set(tlb_command_list "")

    # Wrap tool paths in $<COMMAND_CONFIG> to ensure we use the release tool when building debug
    # targets in a multi-config build, because the debug tool is usually not built by default.
    if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")
        set(cmb "$<COMMAND_CONFIG:")
        set(cme ">")
    else()
        set(cmb "")
        set(cme "")
    endif()

    set(idc_target "${QT_CMAKE_EXPORT_NAMESPACE}::idc")
    set(idc_target_file "$<TARGET_FILE:${idc_target}>")
    set(idc_target_file_command_config_wrapped "${cmb}${idc_target_file}${cme}")

    list(APPEND tlb_command_list
        COMMAND
            "${tool_wrapper}"
            "${idc_target_file_command_config_wrapped}"
            "$<TARGET_FILE:${target}>"
            /idl "${output_idl}" -version 1.0
    )

    list(APPEND tlb_command_list
        COMMAND
            "${tool_wrapper}" midl "${output_idl}" /nologo /tlb "${output_tlb}"
    )

    set(rc_files "$<FILTER:$<TARGET_PROPERTY:${target},SOURCES>,INCLUDE,\\.rc$>")
    set(have_rc_files "$<NOT:$<BOOL:$<STREQUAL:${rc_files},>>>")

    set(no_rc_cmd "echo$<SEMICOLON>No rc-file linked into project. The type library of the \
${target} target will be a separate file.")

    set(idc_args
        "$<SEMICOLON>$<TARGET_FILE:${target}>$<SEMICOLON>/tlb$<SEMICOLON>${output_tlb}")

    # Split command into two parts, so that COMMAND_CONFIG can be applied only to the idc tool path,
    # but not the target and tlb files.
    set(cmd_part1 "${cmb}$<IF:${have_rc_files},${idc_target_file},${no_rc_cmd}>${cme}")
    set(cmd_part2 "$<${have_rc_files}:${idc_args}>")

    list(APPEND tlb_command_list
        COMMAND
            "${tool_wrapper}" "${cmd_part1}" "${cmd_part2}"
    )

    if(NOT arg_NO_AX_SERVER_REGISTRATION AND NOT QT_NO_AX_SERVER_REGISTRATION)
        list(APPEND tlb_command_list
            COMMAND
                "${tool_wrapper}"
                "${idc_target_file_command_config_wrapped}"
                "$<TARGET_FILE:${target}>" /regserver
        )
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        ${tlb_command_list}
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_axserver_executable)
        qt6_add_axserver_executable(${ARGV})
    endfunction()
    function(qt_add_axserver_library)
        qt6_add_axserver_library(${ARGV})
    endfunction()
    function(qt_target_idl)
        qt6_target_idl(${ARGV})
    endfunction()
endif()
