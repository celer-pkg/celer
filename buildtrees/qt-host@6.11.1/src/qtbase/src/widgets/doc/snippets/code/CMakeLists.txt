# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

add_library(widgets_snippets_code OBJECT
    doc_src_layout.cpp
    doc_src_qt4-styles.cpp
)

target_link_libraries(widgets_snippets_code PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

if(QT_FEATURE_itemmodel)
    target_sources(widgets_snippets_code PRIVATE
        doc_src_model-view-programming.cpp
    )
endif()

if(QT_FEATURE_toolbar)
    target_sources(widgets_snippets_code PRIVATE
        doc_src_qt4-mainwindow.cpp
    )
endif()

if(QT_FEATURE_checkbox)
    target_sources(widgets_snippets_code PRIVATE
        doc_src_styles.cpp
    )
endif()

if(QT_FEATURE_lineedit)
    target_sources(widgets_snippets_code PRIVATE
        doc_src_stylesheet.cpp
    )
endif()

if(QT_FEATURE_filedialog)
    target_sources(widgets_snippets_code PRIVATE
        src_gui_dialogs_qfiledialog.cpp
    )
endif()

if(QT_FEATURE_fontdialog)
    target_sources(widgets_snippets_code PRIVATE
        src_gui_dialogs_qfontdialog.cpp
    )
endif()

if(QT_FEATURE_messagebox)
    target_sources(widgets_snippets_code PRIVATE
        src_gui_dialogs_qmessagebox.cpp
    )
endif()

if(QT_FEATURE_wizard)
    target_sources(widgets_snippets_code PRIVATE
        src_gui_dialogs_qwizard.cpp
    )
endif()

set_target_properties(widgets_snippets_code PROPERTIES COMPILE_OPTIONS "-w")

if ("${CMAKE_CXX_COMPILE_FEATURES}" MATCHES "cxx_std_23")
    set_property(TARGET widgets_snippets_code PROPERTY CXX_STANDARD 23)
endif()

set_target_properties(widgets_snippets_code PROPERTIES UNITY_BUILD OFF)
