# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Set the default message latency for the gRPC tests in milliseconds.
add_compile_definitions(QT_GRPC_TEST_MESSAGE_LATENCY=${QT_INTERNAL_GRPC_TEST_MESSAGE_LATENCY_MSEC})

add_subdirectory(client)
add_subdirectory(metadata)
add_subdirectory(channeloptions)
add_subdirectory(http2channel)
add_subdirectory(calloptions)
