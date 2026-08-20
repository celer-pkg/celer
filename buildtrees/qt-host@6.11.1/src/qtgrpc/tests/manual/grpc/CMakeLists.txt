# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt6::qtprotobufgen AND CMAKE_CXX_STANDARD_LATEST GREATER_EQUAL 20)
    add_subdirectory(benchmarks)
endif()
