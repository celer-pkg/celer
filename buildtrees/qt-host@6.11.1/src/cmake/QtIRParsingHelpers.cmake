# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Retrieves the contents of either .git/config or .gitmodules files for further parsing.
function(qt_ir_get_git_config_contents out_var)
    set(options
        READ_GITMODULES
        READ_GIT_CONFIG
        READ_GIT_CONFIG_LOCAL
    )
    set(oneValueArgs
        WORKING_DIRECTORY
    )
    set(multiValueArgs "")
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(arg_READ_GITMODULES)
        set(args -f .gitmodules)
        set(file_message ".gitmodules")
    elseif(arg_READ_GIT_CONFIG)
        set(args "")
        set(file_message ".git/config")
    elseif(arg_READ_GIT_CONFIG_LOCAL)
        set(args "--local")
        set(file_message ".local .git/config")
    else()
        message(FATAL_ERROR "qt_ir_get_git_config_contents: No option specified")
    endif()

    qt_ir_get_working_directory_from_arg(working_directory)

    qt_ir_execute_process_and_log_and_handle_error(
        FORCE_QUIET
        COMMAND_ARGS git config --list ${args}
        OUT_OUTPUT_VAR git_output
        WORKING_DIRECTORY "${working_directory}"
        ERROR_MESSAGE "Failed to get ${file_message} contents for parsing")

    string(STRIP "${git_output}" git_output)
    set(${out_var} "${git_output}" PARENT_SCOPE)
endfunction()

# Parses a git repo url to:
# - check if the given url has a scheme like https:// or git:// or is just a
# relative path with no scheme (possibly containing '../' segments)
# - extracts the scheme if it exists
# - extracts the url without the scheme
function(qt_ir_parse_git_url)
    set(options "")
    set(oneValueArgs
        URL
        OUT_VAR_HAS_URL_SCHEME
        OUT_VAR_SCHEME
        OUT_VAR_URL_WITHOUT_SCHEME
    )
    set(multiValueArgs "")
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REGEX MATCH "^([a-z][a-z0-9+\-.]*://)(.+)" url_scheme_match "${arg_URL}")

    if(url_scheme_match)
        set(has_url_scheme TRUE)
        set(scheme "${CMAKE_MATCH_1}")
        set(url_without_scheme "${CMAKE_MATCH_2}")
    else()
        set(has_url_scheme FALSE)
        set(scheme "")
        set(url_without_scheme "${url}")
    endif()

    if(arg_OUT_VAR_HAS_URL_SCHEME)
        set(${arg_OUT_VAR_HAS_URL_SCHEME} "${has_url_scheme}" PARENT_SCOPE)
    endif()

    if(arg_OUT_VAR_SCHEME)
        set(${arg_OUT_VAR_SCHEME} "${scheme}" PARENT_SCOPE)
    endif()

    if(arg_OUT_VAR_URL_WITHOUT_SCHEME)
        set(${arg_OUT_VAR_URL_WITHOUT_SCHEME} "${url_without_scheme}" PARENT_SCOPE)
    endif()
endfunction()

# Normalizes a url that contains '../' path segments.
# Removes the '../' segments and the directories that they precede.
# Example:
#  git://code.qt.io/qt/../playground/qlitehtml.git
# will be normalized to:
#  git://code.qt.io/playground/qlitehtml.git
function(qt_ir_normalize_git_url url out_var)
    # The exact perl code was while ($base =~ s,(?!\.\./)[^/]+/\.\./,,g) {}
    # That got rid of ../ and ../../ in the path, but it broke down
    # when more than two '../' segments were present.
    #
    # In CMake, we instead parse the url to get the non-scheme suffix,
    # use get_filename_component(ABSOLUTE) to resolve the url as if it was a relative path
    # and then re-add the scheme if it was present.
    qt_ir_parse_git_url(
        URL "${url}"
        OUT_VAR_HAS_URL_SCHEME has_url_scheme
        OUT_VAR_SCHEME url_scheme
        OUT_VAR_URL_WITHOUT_SCHEME url_without_scheme
    )

    # Note the empty BASE_DIR is important, otherwise the path is relative to
    # ${CMAKE_CURRENT_SOURCE_DIR}.
    get_filename_component(normalized_url "${url_without_scheme}" ABSOLUTE BASE_DIR "")

    if(has_url_scheme)
        string(PREPEND normalized_url "${url_scheme}")
    endif()

    set(${out_var} "${normalized_url}" PARENT_SCOPE)
endfunction()

# Parses a key-value line from a .git/config or .gitmodules file
macro(qt_ir_parse_git_key_value)
    string(REGEX REPLACE "^submodule\\.([^.=]+)\\.([^.=]+)=(.*)$" "\\1;\\2;\\3"
        parsed_line "${line}")

    list(LENGTH parsed_line parsed_line_length)
    set(submodule_name "")
    set(key "")
    set(value "")
    if(parsed_line_length EQUAL 3)
        list(GET parsed_line 0 submodule_name)
        list(GET parsed_line 1 key)
        list(GET parsed_line 2 value)
    endif()
endmacro()

# Parses a url line from a .gitmodules file
#   e.g. line - 'submodule.qtbase.url=../qtbase.git'
#
# Arguments
#
# submodule_name
#   submodule name, the key in 'submodule.${submodule_name}.url'
#   e.g. 'qtbase'
# url_value
#   the url where to clone a repo from
#   in perl script it was called $base
#   Examples:
#    - '../qtbase.git'
#    - 'https://code.qt.io/playground/qlitehtml.git'
#    - '../../playground/qlitehtml.git'
# parent_repo_base_git_path
#   the base git path of the parent of the submodule
#   it is either a relative dir or a full url
#   in the perl script it was called $my_repo_base,
#   it was passed as first arg to git_clone_all_submodules,
#   it was passed the value of $subbases{$module} when doing recursive submodule cloning
#   Examples:
#    - 'qt5'
#    - 'tqtc-qt5'
#    - 'qtdeclarative.git'
#    - 'qttools.git'
#    - 'https://code.qt.io/playground/qlitehtml.git'
#
# Outputs
#
# ${out_var_prefix}_${submodule_name}_url
#   just the value of ${url_value}
# ${out_var_prefix}_${submodule_name}_base_git_path
#   the whole url if it has a scheme, otherwise it's the value of
#   ${url_value} relative to ${parent_repo_base_git_path}, so some of the '../' segments
#   are collapsed depending on how many path segments are available in
#   ${parent_repo_base_git_path}.
#   Examples:
#    - 'qtdeclarative.git'
#    - 'https://code.qt.io/playground/qlitehtml.git'
#    - '../playground/qlitehtml.git'
macro(qt_ir_parse_git_url_key out_var_prefix submodule_name url_value parent_repo_base_git_path)
    qt_ir_parse_git_url(
        URL "${url_value}"
        OUT_VAR_HAS_URL_SCHEME has_url_scheme
    )
    if(NOT has_url_scheme)
        set(base_git_path "${parent_repo_base_git_path}/${url_value}")
        qt_ir_normalize_git_url("${base_git_path}" base_git_path)
    else()
        set(base_git_path "${url_value}")
    endif()

    set(${out_var_prefix}_${submodule_name}_url "${url_value}" PARENT_SCOPE)
    set(${out_var_prefix}_${submodule_name}_base_git_path "${base_git_path}" PARENT_SCOPE)
endmacro()

# Parses a .git/config or .gitmodules file contents and sets variables for each submodule
# starting with ${out_var_prefix}_
# These include:
#   ${out_var_prefix}_${submodule_name}_path
#     the path to the submodule relative to the parent repo
#   ${out_var_prefix}_${submodule_name}_branch
#     the branch that should be checked out when the branch option is used
#   ${out_var_prefix}_${submodule_name}_url
#     the url key as encountered in the config
#   ${out_var_prefix}_${submodule_name}_base_git_path
#     the git base path of the submodule, either a full url or a relative path
#   ${out_var_prefix}_${submodule_name}_update
#     the status of the submodule, can be 'none'
#   ${out_var_prefix}_${submodule_name}_status
#     the status of the submodule, can be 'essential', 'addon', etc
#   ${out_var_prefix}_${submodule_name}_depends
#     the list of submodules that this submodule depends on
#   ${out_var_prefix}_${submodule_name}_recommends
#     the list of submodules that this submodule recommends to be used with
#   ${out_var_prefix}_submodules
#     a list of all known submodule names encountered in the file
#   ${out_var_prefix}_submodules_to_remove
#     a list of all submodules to remove due to update == 'none'
#   ${out_var_prefix}_statuses to
#     a list of all known submodule statuses like 'essential', 'addon', etc
#   ${out_var_prefix}_status_${status}_submodules
#     a list of all submodules with the specific status
function(qt_ir_parse_git_config_file_contents out_var_prefix)
    set(options
        READ_GITMODULES
        READ_GIT_CONFIG
        READ_GIT_CONFIG_LOCAL
    )
    set(oneValueArgs
        PARENT_REPO_BASE_GIT_PATH
        WORKING_DIRECTORY
    )
    set(multiValueArgs "")
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qt_ir_get_working_directory_from_arg(working_directory)

    if(NOT arg_PARENT_REPO_BASE_GIT_PATH)
        message(FATAL_ERROR
            "qt_ir_parse_git_config_file_contents: No base PARENT_REPO_BASE_GIT_PATH specified")
    endif()
    set(parent_repo_base_git_path "${arg_PARENT_REPO_BASE_GIT_PATH}")

    if(arg_READ_GITMODULES)
        set(read_git_config READ_GITMODULES)
    elseif(arg_READ_GIT_CONFIG)
        set(read_git_config READ_GIT_CONFIG)
    elseif(arg_READ_GIT_CONFIG_LOCAL)
        set(read_git_config READ_GIT_CONFIG_LOCAL)
    else()
        message(FATAL_ERROR
            "qt_ir_parse_gitmodules_file_contents: No valid git config file specified")
    endif()

    qt_ir_get_git_config_contents(contents
        ${read_git_config}
        WORKING_DIRECTORY "${working_directory}"
    )
    string(REPLACE "\n" ";" lines "${contents}")

    set(known_submodules "")
    set(statuses "")
    set(submodules_to_remove "")

    foreach(line IN LISTS lines)
        qt_ir_parse_git_key_value()
        if(NOT submodule_name OR NOT key OR value STREQUAL "")
            continue()
        endif()

        list(APPEND known_submodules "${submodule_name}")

        if(key STREQUAL "path")
            set(${out_var_prefix}_${submodule_name}_path "${value}" PARENT_SCOPE)
        elseif(key STREQUAL "branch")
            set(${out_var_prefix}_${submodule_name}_branch "${value}" PARENT_SCOPE)
        elseif(key STREQUAL "url")
            qt_ir_parse_git_url_key(
                "${out_var_prefix}" "${submodule_name}" "${value}" "${parent_repo_base_git_path}")
        elseif(key STREQUAL "update")
            # Some repo submodules had a update = none key in their .gitmodules
            # in which case we're supposed to skip initialzing those submodules,
            # which the perl script did by adding -${submodule_name} to the subset.
            # See qtdeclarative Change-Id: I633404f1c00d83dcbdca06a1d287623190323028
            set(${out_var_prefix}_${submodule_name}_update "${value}" PARENT_SCOPE)
            if(value STREQUAL "none")
                list(APPEND submodules_to_remove "-${submodule_name}")
            endif()
        elseif(key STREQUAL "status")
            set(status_submodules "${${out_var_prefix}_status_${value}_submodules}")
            list(APPEND status_submodules "${submodule_name}")
            list(REMOVE_DUPLICATES status_submodules)
            list(APPEND statuses "${value}")

            set(${out_var_prefix}_status_${value}_submodules "${status_submodules}")
            set(${out_var_prefix}_status_${value}_submodules "${status_submodules}" PARENT_SCOPE)
            set(${out_var_prefix}_${submodule_name}_status "${value}" PARENT_SCOPE)
        elseif(key STREQUAL "depends")
            string(REPLACE " " ";" value "${value}")
            set(${out_var_prefix}_${submodule_name}_depends "${value}" PARENT_SCOPE)
        elseif(key STREQUAL "recommends")
            string(REPLACE " " ";" value "${value}")
            set(${out_var_prefix}_${submodule_name}_recommends "${value}" PARENT_SCOPE)
        endif()
    endforeach()

    list(REMOVE_DUPLICATES known_submodules)
    list(REMOVE_DUPLICATES submodules_to_remove)
    list(REMOVE_DUPLICATES statuses)
    set(${out_var_prefix}_submodules "${known_submodules}" PARENT_SCOPE)
    set(${out_var_prefix}_submodules_to_remove "${submodules_to_remove}" PARENT_SCOPE)
    set(${out_var_prefix}_statuses "${statuses}" PARENT_SCOPE)
endfunction()
