#! /bin/sh

# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#
# This is a small script to copy the required files from a openxr tarball
# into 3rdparty/openxr/ . Documentation, tests, demos etc. are not imported.

if [ $# -ne 2 ]; then
    echo "Usage: $0 openxr_tarball_dir/ \$QTDIR/src/3rdparty/openxr/"
    exit 1
fi

SRC_DIR=$1
TARGET_DIR=$2

if [ ! -d "$SRC_DIR" -o ! -r "$SRC_DIR" -o ! -d "$TARGET_DIR" -o ! -w "$TARGET_DIR" ]; then
    echo "Either the openxr source dir or the target dir do not exist,"
    echo "are not directories or have the wrong permissions."
    exit 2
fi

# with 1 argument, copies SRC_DIR/$1 to TARGET_DIR/$1
# with 2 arguments, copies SRC_DIR/$1 to TARGET_DIR/$2
copy_file_or_dir() {
    if [ $# -lt 1 -o $# -gt 2  ]; then
        echo "Wrong number of arguments to copy_file_or_dir"
        exit 3
    fi

    SOURCE_FILE=$1
    if [ -n "$2" ]; then
        DEST_FILE=$2
    else
        DEST_FILE=$1
    fi

    mkdir -p "$TARGET_DIR/$(dirname "$SOURCE_FILE")"
    cp -R "$SRC_DIR/$SOURCE_FILE" "$TARGET_DIR/$DEST_FILE"
}

FILES="
    README.md
    LICENSE
    include/openxr/CMakeLists.txt
    include/openxr/openxr.h
    include/openxr/openxr_loader_negotiation.h
    include/openxr/openxr_platform.h
    include/openxr/openxr_platform_defines.h
    include/openxr/openxr_reflection.h
    include/openxr/openxr_reflection_parent_structs.h
    include/openxr/openxr_reflection_structs.h
    src/xr_generated_dispatch_table.h
    src/xr_generated_dispatch_table.c
    src/xr_generated_dispatch_table_core.h
    src/xr_generated_dispatch_table_core.c
    src/common/android_logging.h
    src/common/extra_algorithms.h
    src/common/filesystem_utils.cpp
    src/common/filesystem_utils.hpp
    src/common/hex_and_handles.h
    src/common/object_info.cpp
    src/common/object_info.h
    src/common/platform_utils.hpp
    src/common/stdfs_conditions.h
    src/common/unique_asset.h
    src/common/vulkan_debug_object_namer.hpp
    src/common/xr_dependencies.h
    src/common/xr_linear.h
    src/loader/android_utilities.cpp
    src/loader/android_utilities.h
    src/loader/api_layer_interface.cpp
    src/loader/api_layer_interface.hpp
    src/loader/exception_handling.hpp
    src/loader/loader_core.cpp
    src/loader/loader_init_data.cpp
    src/loader/loader_init_data.hpp
    src/loader/loader_instance.cpp
    src/loader/loader_instance.hpp
    src/loader/loader_logger.cpp
    src/loader/loader_logger.hpp
    src/loader/loader_logger_recorders.cpp
    src/loader/loader_logger_recorders.hpp
    src/loader/loader_platform.hpp
    src/loader/loader_properties.cpp
    src/loader/loader_properties.hpp
    src/loader/manifest_file.cpp
    src/loader/manifest_file.hpp
    src/loader/runtime_interface.cpp
    src/loader/runtime_interface.hpp
    src/loader/xr_generated_loader.cpp
    src/loader/xr_generated_loader.hpp
    src/external/jsoncpp/LICENSE
    src/external/jsoncpp/README.md
    src/external/jsoncpp/src/lib_json/json_reader.cpp
    src/external/jsoncpp/src/lib_json/json_value.cpp
    src/external/jsoncpp/src/lib_json/json_writer.cpp
    src/external/jsoncpp/src/lib_json/json_tool.h
    src/external/jsoncpp/src/lib_json/json_valueiterator.inl
    src/external/jsoncpp/include/json/allocator.h
    src/external/jsoncpp/include/json/assertions.h
    src/external/jsoncpp/include/json/config.h
    src/external/jsoncpp/include/json/forwards.h
    src/external/jsoncpp/include/json/json.h
    src/external/jsoncpp/include/json/json_features.h
    src/external/jsoncpp/include/json/reader.h
    src/external/jsoncpp/include/json/value.h
    src/external/jsoncpp/include/json/version.h
    src/external/jsoncpp/include/json/writer.h
    src/external/android-jni-wrappers/wrap/ObjectWrapperBase.h
    src/external/android-jni-wrappers/wrap/android.content.cpp
    src/external/android-jni-wrappers/wrap/android.content.h
    src/external/android-jni-wrappers/wrap/android.content.impl.h
    src/external/android-jni-wrappers/wrap/android.database.cpp
    src/external/android-jni-wrappers/wrap/android.database.h
    src/external/android-jni-wrappers/wrap/android.database.impl.h
    src/external/android-jni-wrappers/wrap/android.net.cpp
    src/external/android-jni-wrappers/wrap/android.net.h
    src/external/jnipp/LICENSE
    src/external/jnipp/README.md
    src/external/jnipp/jnipp.h
    src/external/jnipp/jnipp.cpp
"

for i in $FILES; do
    copy_file_or_dir "$i"
done
