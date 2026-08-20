# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This project builds and installs the client library generated from the spec,
# and the public headers created for the client.

find_package(Qt6 REQUIRED COMPONENTS Core Network Test OpenApi OpenApiCommon)

# We need to set this explicitly, so it gets baked into the installed client library,
# because it does not get processed by our deployment code.
# Otherwise the application in the other project would fail to find the dependencies of the
# client library at runtime on Linux.
set(CMAKE_INSTALL_RPATH "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_LIBS}")

qt_standard_project_setup(REQUIRES 6.10)

# Build the libraries
qt_add_library(CompressionClient)
qt6_add_openapi_client(CompressionClient
    SPEC_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/compression.yaml"
    OUTPUT_PUBLIC_HEADERS_DIR
        client_public_headers_dir
    OUTPUT_PRIVATE_HEADERS_DIR
        client_private_headers_dir
)

# Propagate the installed include dirs for any consuming project.
target_include_directories(CompressionClient INTERFACE
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/private>"
)

# Install the prebuilt libraries
install(
    TARGETS CompressionClient
    EXPORT CompressionClientTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the cmake files to find the libraries in another project
install(EXPORT
    CompressionClientTargets
    NAMESPACE CompressionClient::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CompressionClient"
)

# Create a helper file that includes the above cmake file via full path, so it's easier to
# consume in the other project
set(rel_targets_path "cmake/CompressionClient/CompressionClientTargets.cmake")
set(full_targets_path "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${rel_targets_path}")
set(include_file_path "${CMAKE_TARGETS_INCLUDE_PATH}")
file(GENERATE OUTPUT "${include_file_path}"
    CONTENT "include(\"${full_targets_path}\")\n"
)

# Install the public and private headers for the libraries.
# Slash at the end is important to install the files, and not the directory.
install(
    DIRECTORY "${client_public_headers_dir}/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
    DIRECTORY "${client_private_headers_dir}/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/private"
)
