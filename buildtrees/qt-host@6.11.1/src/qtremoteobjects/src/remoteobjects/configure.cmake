# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries



#### Tests



#### Features

qt_feature("use_ham" PUBLIC
    LABEL "High Availability Manager (ham)"
    PURPOSE "Use QNX's High Availability Manager (ham) library"
    AUTODETECT OFF
    CONDITION QNX
)
qt_feature_definition("use_ham" "QT_NO_USE_HAM" NEGATE VALUE "1")
qt_configure_add_summary_section(NAME "Qt Remote Objects")
qt_configure_add_summary_entry(ARGS "use_ham")
qt_configure_end_summary_section() # end of "Qt Remote Objects" section
