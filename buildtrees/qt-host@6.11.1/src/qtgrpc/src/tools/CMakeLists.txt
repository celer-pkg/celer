# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_skip_sanitizer()

# Skip building qtprotoccommon when not building tools
if((QT_FEATURE_qtprotobufgen OR QT_FEATURE_qtgrpcgen) AND
    TARGET protobuf::libprotoc AND TARGET protobuf::libprotobuf)
    add_subdirectory(qtprotoccommon)
endif()

if(QT_FEATURE_qtprotobufgen OR
    (QT_HOST_PATH AND Qt6::qtprotobufgen IN_LIST Qt6ProtobufTools_TARGETS
     AND NOT QT_FORCE_BUILD_TOOLS))

    add_subdirectory(qtprotobufgen)
elseif(NOT QT_FEATURE_qtprotobufgen AND QT_FORCE_BUILD_TOOLS)
    # TODO: We ignore the QT_FORCE_BUILD_TOOLS option for this tool to pass the CI check.
    # See QTBUG-110849 for details.
    message(WARNING "You are trying to force build the qtprotobufgen tool, but its dependencies"
        " are missing. The tool build is disabled.")
endif()

if(QT_FEATURE_qtgrpcgen OR
    (QT_HOST_PATH AND Qt6::qtgrpcgen IN_LIST Qt6GrpcTools_TARGETS
     AND NOT QT_FORCE_BUILD_TOOLS))
    add_subdirectory(qtgrpcgen)
elseif(NOT QT_FEATURE_qtprotobufgen AND QT_FORCE_BUILD_TOOLS)
    # TODO: We ignore the QT_FORCE_BUILD_TOOLS option for this tool to pass the CI check.
    # See QTBUG-110849 for details.
    message(WARNING "You are trying to force build the qtgrpcgen tool, but its dependencies"
        " are missing. The tool build is disabled.")
endif()

# Needs to be called at the end after all relevant target have created.
qt_internal_skip_linking_sanitizer()
