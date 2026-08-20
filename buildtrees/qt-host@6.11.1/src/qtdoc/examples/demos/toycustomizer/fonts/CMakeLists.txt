# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(assets_fonts
    WinkySans/WinkySans-VariableFont_wght.ttf
    WinkySans/WinkySans-Italic-VariableFont_wght.ttf
    DynaPuff/DynaPuff-VariableFont_wdth,wght.ttf
)

qt_add_resources(${TARGET_NAME} "fonts"
    PREFIX "/qt/qml/ToyCustomizer/qml/fonts/"
    FILES ${assets_fonts}
)

set(clean_fonts
    "${assets_fonts}"
    "DynaPuff/OFL.txt"
    "WinkySans/OFL.txt"
)
list(TRANSFORM clean_fonts PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${clean_fonts})
