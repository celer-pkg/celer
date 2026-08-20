# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qquickwebview Binary:
#####################################################################

qt_internal_add_test(tst_qquickwebview
    SOURCES
        util.h testwindow.h
        tst_qquickwebview.cpp
    LIBRARIES
        Qt::Quick
        Qt::WebViewPrivate
        Qt::WebViewQuickPrivate
)

set(testdata_resource_files
    "html/basic_page.html"
    "html/basic_page2.html"
    "html/direct-image-compositing.html"
    "html/inputmethod.html"
    "html/scroll.html"
    "html/cookies.html"
    "html/javascript.html"
)

qt_internal_add_resource(tst_qquickwebview "testdata"
    PREFIX
        "/"
    BASE
        "html"
    FILES
        ${testdata_resource_files}
)
set(testdata1_resource_files
    "html/resources/simple_image.png"
)

qt_internal_add_resource(tst_qquickwebview "testdata1"
    PREFIX
        "/resources"
    BASE
        "html"
    FILES
        ${testdata1_resource_files}
)

