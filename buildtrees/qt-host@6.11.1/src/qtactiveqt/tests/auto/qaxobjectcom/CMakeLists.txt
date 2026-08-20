# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Configure the manifest file to point to the com_server dll
# because the dll name is different in debug and release
set(TestServerDll "TestServer${CMAKE_DEBUG_POSTFIX}.dll")
configure_file(
    tst_qaxobjectcom.exe.manifest.in
    tst_qaxobjectcom.exe.manifest
)

qt_internal_add_test(tst_qaxobjectcom
    SOURCES
        tst_qaxobjectcom.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/tst_qaxobjectcom.exe.manifest
    LIBRARIES
        Qt::AxContainer
        Qt::CorePrivate
        Qt::AxBasePrivate
)

# Embed the manifest file into the binary for registry free COM
target_link_options(tst_qaxobjectcom PRIVATE "/MANIFEST")

# Build the com_server automatically
add_subdirectory(testserver)
add_dependencies(tst_qaxobjectcom testserver)

# Generate qaxobject COM wrapper for our COM test server
qt6_target_typelibs(tst_qaxobjectcom
    LIBRARIES
        ${CMAKE_CURRENT_BINARY_DIR}/testserver/testserverlib.tlb
)
