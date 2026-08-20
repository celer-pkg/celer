# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_qml_module(quotesInPath
    URI quotesInPath
    IMPORT_PATH "\"hello\"\"world\""
    QML_FILES Main.qml
)
