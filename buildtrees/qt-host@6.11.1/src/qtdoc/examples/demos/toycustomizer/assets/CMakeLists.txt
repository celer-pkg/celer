# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(asset_url "https://ordp.qt.io/qt/bundles/toycustomizer/toy-customizer-assets.zip")
set(asset_file "${CMAKE_BINARY_DIR}/.assets/toy-customizer-assets.zip")
set(asset_extract_dir "${CMAKE_SOURCE_DIR}")
set(asset_contents animations fonts images meshes)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${asset_file})

if(NOT EXISTS ${asset_file})
  message(STATUS "Downloading asset file...")
  file(DOWNLOAD
    ${asset_url} ${asset_file}
    STATUS asset_status
    TLS_VERIFY ON
  )
  list(GET asset_status 0 asset_status_code)
  if (asset_status_code EQUAL 0)
    message(STATUS "Downloading asset file done.")
  else()
    if(EXISTS "${asset_file}")
      file(REMOVE "${asset_file}")
    endif()
    list(GET asset_status 1 asset_status_message)
    message(FATAL_ERROR "Downloading asset file failed:\n  error: ${asset_status_message}")
  endif()
else()
  message(STATUS "Download asset file skipped: already exists.")
endif()

# Asset extraction is performed here because accidentally removing an asset file
# causes the configuration step to fail. Even running 'clean' will not resolve
# this issue. The failure can only be fixed by deleting the downloaded asset file.
# Since identifying this cause is difficult, keeping the extraction step here
# ensures that assets are re-extracted each time the example is built.
message(STATUS "Extracting asset file...")
file(MAKE_DIRECTORY ${asset_extract_dir})
file(ARCHIVE_EXTRACT
  INPUT ${asset_file}
  DESTINATION ${asset_extract_dir}
)
message(STATUS "Extracting asset file done.")
