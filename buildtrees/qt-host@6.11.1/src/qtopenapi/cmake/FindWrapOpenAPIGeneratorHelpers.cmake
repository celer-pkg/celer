# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(_qt_internal_openapi_find_cli_jar_from_pip out_var_jar_path)
    # Check whether the detected CLI was installed via pip
    execute_process(
        COMMAND python3 -c "import openapi_generator_cli"
        RESULT_VARIABLE import_openapi_generator_result
        ERROR_QUIET
    )

    if(NOT import_openapi_generator_result)
        # Get the jar path specified in __init__.py
        string(CONCAT python_command
            "import openapi_generator_cli, importlib.resources;"
            "print(importlib.resources.files('openapi_generator_cli') / "
            "'openapi-generator.jar')"
        )
        execute_process(
            COMMAND python3 -c "${python_command}"
            OUTPUT_VARIABLE jar_path
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE python_result
        )
        if(NOT python_result AND EXISTS "${jar_path}")
            set(${out_var_jar_path} "${jar_path}" PARENT_SCOPE)
            message(DEBUG "Found OpenAPI Generator JAR: ${jar_path}")
        else()
            message(DEBUG
                "Could not locate the JAR file in 'openapi_generator_cli' python package.")
        endif()
    endif()
endfunction()

function(_qt_internal_openapi_find_cli_jar_from_brew out_var_jar_path)
    execute_process(
        COMMAND brew --prefix openapi-generator
        OUTPUT_VARIABLE brew_openapi_generator_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE brew_result
    )

    if(NOT brew_result)
        # prefix should end with slash in order for 'find' to work properly
        if(NOT brew_openapi_generator_prefix MATCHES "\/$")
            string(APPEND brew_openapi_generator_prefix "/")
        endif()

        execute_process(
            COMMAND find "${brew_openapi_generator_prefix}" -name openapi-generator-cli.jar
            OUTPUT_VARIABLE jar_path
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE find_result
        )

        if(NOT find_result AND EXISTS "${jar_path}")
            set(${out_var_jar_path} "${jar_path}" PARENT_SCOPE)
            message(DEBUG "Found OpenAPI Generator JAR: ${jar_path}")
        else()
            message(DEBUG
                "Could not locate OpenAPI Generator JAR under Homebrew prefix:
                ${brew_openapi_generator_prefix}.")
        endif()
    endif()
endfunction()

function(_qt_internal_openapi_find_cli_jar_from_scoop out_var_jar_path)
    execute_process(
        COMMAND powershell -NoProfile -Command "scoop which openapi-generator-cli"
        OUTPUT_VARIABLE current_cli_path
        RESULT_VARIABLE scoop_result
    )
    if(NOT scoop_result)
        # Replace ~ with home directory because it is not expanded on windows.
        if(current_cli_path MATCHES "^~")
            # Get and normalize the home directory and current cli path
            set(home_dir "$ENV{USERPROFILE}")
            file(TO_CMAKE_PATH "${home_dir}" home_dir)
            file(TO_CMAKE_PATH "${current_cli_path}" current_cli_path)

            string(REGEX REPLACE "^~" "${home_dir}"
                current_cli_path "${current_cli_path}")
        endif()

        get_filename_component(current_cli_dir "${current_cli_path}" DIRECTORY)
        set(jar_path "${current_cli_dir}/openapi-generator-cli.jar")

        if(EXISTS "${jar_path}")
            set(${out_var_jar_path} "${jar_path}" PARENT_SCOPE)
            message(DEBUG "Found OpenAPI Generator JAR: ${jar_path}")
        else()
            message(DEBUG "Could not locate OpenAPI Generator JAR at: ${jar_path}.")
        endif()
    endif()
endfunction()

function(_qt_internal_openapi_find_cli_jar out_var_jar_path)
    set(jar_path "")
    _qt_internal_openapi_find_cli_jar_from_pip(jar_path)

    if(NOT jar_path)
        if(CMAKE_HOST_APPLE)
            _qt_internal_openapi_find_cli_jar_from_brew(jar_path)
        elseif(CMAKE_HOST_WIN32)
            _qt_internal_openapi_find_cli_jar_from_scoop(jar_path)
        endif()
    endif()

    if(jar_path)
        set(${out_var_jar_path} "${jar_path}" CACHE FILEPATH "Path to OpenAPI Generator JAR" FORCE)
    endif()
endfunction()

function(_qt_internal_check_custom_generator_support out_var_custom_generator_support)
    set(help_command "${OPENAPI_GENERATOR_CLI_EXECUTABLE}" help)
    if(CMAKE_HOST_WIN32)
        list(PREPEND help_command cmd /c)
    endif()

    # Only npm has --custom-generator option
    execute_process(
        COMMAND ${help_command}
        OUTPUT_VARIABLE cli_help_output
        ERROR_QUIET
    )
    string(FIND "${cli_help_output}" "--custom-generator" has_custom_generator)

    if(has_custom_generator EQUAL -1)
        set(${out_var_custom_generator_support} FALSE PARENT_SCOPE)
    else()
        set(${out_var_custom_generator_support} TRUE PARENT_SCOPE)
    endif()
endfunction()
