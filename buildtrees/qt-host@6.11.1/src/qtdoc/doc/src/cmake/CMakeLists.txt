# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case skip regeneration

add_library(qtcmake INTERFACE)

qt_internal_add_docs(qtcmake
                     qtcmake.qdocconf
)
