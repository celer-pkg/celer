# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_generated_lsp_types
    SOURCES
        tst_generated_lsp_types.h tst_generated_lsp_types.cpp
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::LanguageServerPrivate
        Qt::Test
)
