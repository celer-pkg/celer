# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_library(PetStoreClient)
qt_add_openapi_client(PetStoreClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/petstore.yaml
)
qt_autogen_tools_initial_setup(PetStoreClient)

if(TARGET Qt::Gui)
    add_subdirectory(petapitest)
endif()
add_subdirectory(storeapitest)
add_subdirectory(userapitest)
