# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_feature("urischeme_replyhandler" PRIVATE
    LABEL "URI Scheme Reply Handler"
    CONDITION QT_FEATURE_gui
)

qt_configure_add_summary_section(NAME "Qt NetworkAuth")
qt_configure_add_summary_entry(ARGS "urischeme_replyhandler")
qt_configure_end_summary_section()
