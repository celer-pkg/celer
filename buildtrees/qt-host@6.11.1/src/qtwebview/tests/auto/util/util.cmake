# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT TARGET WebViewTest::Util)
   add_library(qwebviewtestutil INTERFACE)
   target_include_directories(qwebviewtestutil INTERFACE ${CMAKE_CURRENT_LIST_DIR})
   add_library(WebViewTest::Util ALIAS qwebviewtestutil)
endif()
