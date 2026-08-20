# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#
# CMake script to collect and stage modified .ts files to git
#

if (NOT LCONVERT_BIN)
    message(FATAL_ERROR "lconvert binary not specified. Use LCONVERT_BIN to pass in a value")
endif()


set(output_file ".git_stage_files.txt")

execute_process(
    COMMAND git diff-files --name-only "*.ts"
    OUTPUT_FILE ${output_file}
    RESULT_VARIABLE git_result
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT git_result EQUAL 0)
    message(FATAL_ERROR "'git diff-files --name-only *.ts' exited with non-zero status.")
endif()

file(STRINGS ${output_file} file_list)
foreach (file IN LISTS file_list)
    execute_process(
        COMMAND ${LCONVERT_BIN} -locations none -i "${file}" -o "${file}"
        RESULT_VARIABLE lconvert_result
        COMMAND_ECHO STDOUT
    )
    if (NOT lconvert_result EQUAL 0)
        message(FATAL_ERROR "Command exited with non-zero status.")
    endif()
endforeach()

execute_process(
    COMMAND git add ${file_list}
    RESULT_VARIABLE git_result
    COMMAND_ECHO STDOUT
)

if (NOT git_result EQUAL 0)
    message(FATAL_ERROR "Command exited with non-zero status.")
endif()

if (file_list)
    message("Translation files added to git. Please commit them to finish.")
endif()
