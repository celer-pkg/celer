# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_protobuf_qtcoretypes)
    add_subdirectory(protobufqtcoretypes)
endif()

if(TARGET Qt6::Gui AND QT_FEATURE_protobuf_qtguitypes)
    add_subdirectory(protobufqtguitypes)
endif()
