# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG_save "${CMAKE_FIND_PACKAGE_PREFER_CONFIG}")
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

qt_find_package(protobuf
    MODULE_NAME global
    NAMES protobuf Protobuf
    VCPKG_PORT protobuf
)
if(NOT protobuf_FOUND)
    # Attempt looking in MODULE mode
    qt_find_package(Protobuf
        MODULE_NAME global
    )
endif()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG "${CMAKE_FIND_PACKAGE_PREFER_CONFIG_save}")

# WrapProtoc::WrapProtoc could come from top-level CMakeLists.txt so avoid promoting it to GLOBAL
# here in this case.
if(TARGET WrapProtoc::WrapProtoc)
    qt_internal_disable_find_package_global_promotion(WrapProtoc::WrapProtoc)
endif()
qt_find_package(WrapProtoc
    PROVIDED_TARGETS WrapProtoc::WrapProtoc
    MODULE_NAME global
)

qt_config_compile_test(libprotobuf
    LIBRARIES
        protobuf::libprotobuf
    CODE
"#include <google/protobuf/descriptor.h>

int main(void)
{
    google::protobuf::DescriptorPool pool;
    pool.FindMessageTypeByName(\"\");
    return 0;
}
")

qt_config_compile_test(libprotoc
    LIBRARIES
        protobuf::libprotoc
        protobuf::libprotobuf
    CODE
"#include <google/protobuf/compiler/plugin.h>

int main(void)
{
    return ::google::protobuf::compiler::PluginMain(0, nullptr, nullptr);
}
")

qt_feature("qtprotobufgen" PRIVATE
    SECTION "Utilities"
    LABEL "Qt Protobuf generator"
    PURPOSE "Provides support for generating Qt-based classes for use with Protocol Buffers."
    CONDITION TARGET protobuf::libprotoc AND TARGET protobuf::libprotobuf AND
        TARGET WrapProtoc::WrapProtoc AND TEST_libprotobuf AND TEST_libprotoc
)

qt_feature("protobuf-qtcoretypes" PUBLIC
    SECTION "Qt Protobuf"
    LABEL "Qt Core types support"
    AUTODETECT TRUE
    CONDITION TARGET WrapProtoc::WrapProtoc AND
        ( TARGET Qt6::qtprotobufgen OR Qt6::qtprotobufgen IN_LIST Qt6ProtobufTools_TARGETS OR
            ( QT_FEATURE_qtprotobufgen AND NOT CMAKE_CROSSCOMPILING ) )
)

qt_feature("protobuf-qtguitypes" PUBLIC
    SECTION "Qt Protobuf"
    LABEL "Qt Gui types support"
    AUTODETECT TRUE
    CONDITION TARGET WrapProtoc::WrapProtoc AND
        ( TARGET Qt6::qtprotobufgen OR Qt6::qtprotobufgen IN_LIST Qt6ProtobufTools_TARGETS OR
            ( QT_FEATURE_qtprotobufgen AND NOT CMAKE_CROSSCOMPILING ) )
)

qt_feature("protobuf-wellknowntypes" PUBLIC
    SECTION "Qt Protobuf"
    LABEL "Well-known types support"
    AUTODETECT TRUE
    CONDITION TARGET WrapProtoc::WrapProtoc AND
        (TARGET protobuf::libprotobuf OR QT_PROTOBUF_WELL_KNOWN_TYPES_PROTO_DIR) AND
        ( TARGET Qt6::qtprotobufgen OR Qt6::qtprotobufgen IN_LIST Qt6ProtobufTools_TARGETS OR
            ( QT_FEATURE_qtprotobufgen AND NOT CMAKE_CROSSCOMPILING ) )
)

qt_feature("protobufquick" PUBLIC
    SECTION "Qt Protobuf"
    LABEL "QML Protobuf support"
    PURPOSE "Allows using the generated Protobuf code from QML"
    AUTODETECT TRUE
    CONDITION TARGET Qt6::Quick
)

qt_feature("protobuf-unsafe-registry" PRIVATE
    SECTION "Qt Protobuf"
    LABEL "Unsafe registry"
    PURPOSE "Allows lock-free registry access for custom serializers"
    AUTODETECT FALSE
)

qt_configure_add_summary_section(NAME "Qt Protobuf")
qt_configure_add_summary_entry(ARGS "protobuf-qtcoretypes")
qt_configure_add_summary_entry(ARGS "protobuf-qtguitypes")
qt_configure_add_summary_entry(ARGS "protobuf-wellknowntypes")
qt_configure_add_summary_entry(ARGS "protobufquick")
qt_configure_add_summary_entry(ARGS "protobuf-unsafe-registry")
qt_configure_end_summary_section()
qt_configure_add_summary_section(NAME "Qt Protobuf tools")
qt_configure_add_summary_entry(ARGS "qtprotobufgen")
qt_configure_end_summary_section()
