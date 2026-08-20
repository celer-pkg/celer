# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qwebview Test:
#####################################################################

include(../../util/util.cmake)

qt_internal_add_test(tst_qwebview
    SOURCES
        tst_qwebview.cpp
    LIBRARIES
        Qt::Gui
        Qt::WebViewPrivate
        WebViewTest::Util
)

set(resource_files
    "html/basic_page.html"
    "html/basic_page2.html"
)

qt_internal_add_resource(tst_qwebview "testdata"
    PREFIX
        "/"
    BASE
        "html"
    FILES
        ${resource_files}
)
