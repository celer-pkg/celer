# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries



#### Tests



#### Features

qt_feature("bluetooth" PUBLIC
    LABEL "Qt Bluetooth"
    PURPOSE "The Bluetooth API provides connectivity between Bluetooth enabled devices."
    CONDITION TARGET Qt::Network
)
qt_feature("nfc" PUBLIC
    LABEL "Qt NFC"
    PURPOSE "The NFC API provides connectivity between NFC enabled devices."
)

qt_configure_add_summary_section(NAME "Qt Connectivity")
qt_configure_add_summary_entry(ARGS "bluetooth")
qt_configure_add_summary_entry(ARGS "nfc")
qt_configure_end_summary_section() # end of "Qt Connectivity" section
