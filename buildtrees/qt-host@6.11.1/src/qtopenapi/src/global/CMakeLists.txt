# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_feature_module_begin(NO_MODULE
    PUBLIC_FILE fake_openapi_config.h
    PRIVATE_FILE fake_openapi_config_p.h
)
include("${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake")
qt_feature_module_end(NO_MODULE)
