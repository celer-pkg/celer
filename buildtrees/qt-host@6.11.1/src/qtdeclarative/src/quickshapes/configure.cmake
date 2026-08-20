# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Features

qt_feature("quickshapes-designhelpers" PRIVATE
    SECTION "Qt Quick Shapes"
    LABEL "Design Helpers"
    PURPOSE "Provides helper types based on Qt Quick Shapes for use in design tools like Figma."
    CONDITION QT_FEATURE_quick_path
)
qt_configure_add_summary_section(NAME "Qt Quick Shapes")
qt_configure_add_summary_entry(ARGS "quickshapes-designhelpers")
qt_configure_end_summary_section() # end of "Qt Quick Shapes" section
