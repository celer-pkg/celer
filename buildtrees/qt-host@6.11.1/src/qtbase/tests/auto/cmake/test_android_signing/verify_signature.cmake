# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT KEYTOOL_PATH)
    message(FATAL_ERROR "KEYTOOL_PATH is missing.")
endif()

if(NOT OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is missing.")
endif()

execute_process(
    COMMAND ${KEYTOOL_PATH} -printcert -jarfile ${OUTPUT_FILE}
    RESULT_VARIABLE keytool_result
    OUTPUT_VARIABLE keytool_output
    ERROR_VARIABLE keytool_error
)

if(NOT keytool_result EQUAL 0)
    message(FATAL_ERROR "Keytool command '${KEYTOOL_PATH}' -printcert -jarfile '${OUTPUT_FILE}'"
        "failed with error: ${keytool_error} ${keytool_output}")
endif()

if(NOT keytool_output MATCHES ".+Certificate #1:.+")
    message(FATAL_ERROR "Package ${OUTPUT_FILE} is not signed.")
endif()
