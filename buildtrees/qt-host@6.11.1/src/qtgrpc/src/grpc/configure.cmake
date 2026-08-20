# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
qt_feature("grpc" PUBLIC
    SECTION "Utilities"
    LABEL "gRPC support"
    PURPOSE "Provides support for serializing and deserializing gRPC services using Qt."
    CONDITION TARGET Qt6::Network AND QT_FEATURE_http
)

qt_feature("qtgrpcgen" PRIVATE
    SECTION "Utilities"
    LABEL "Qt GRPC generator"
    PURPOSE "Provides support for generating Qt-based gRPC services."
    DISABLE NOT QT_FEATURE_grpc
    CONDITION TARGET protobuf::libprotoc AND TARGET protobuf::libprotobuf AND
        TARGET WrapProtoc::WrapProtoc AND TEST_libprotobuf AND TEST_libprotoc
)

qt_feature("grpcquick" PUBLIC
    SECTION "Utilities"
    LABEL "QML gRPC support"
    PURPOSE "Allows using the gRPC API from QML"
    CONDITION QT_FEATURE_grpc AND TARGET Qt6::Qml
)

qt_configure_add_summary_section(NAME "Qt GRPC")
qt_configure_add_summary_entry(ARGS "grpc")
qt_configure_add_summary_entry(ARGS "grpcquick")
qt_configure_end_summary_section()
qt_configure_add_summary_section(NAME "Qt GRPC tools")
qt_configure_add_summary_entry(ARGS "qtgrpcgen")
qt_configure_end_summary_section()
