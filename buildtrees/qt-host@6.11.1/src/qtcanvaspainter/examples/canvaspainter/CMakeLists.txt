# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Gui)
    qt_internal_add_example(compacthealth)
endif()

if(TARGET Qt::Quick)
    qt_internal_add_example(gallery)
endif()

if(TARGET Qt::Widgets)
    qt_internal_add_example(hellowidget)
endif()

foreach(target IN LISTS reused_dir_targets)
    if(TARGET ${target})
        qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS moc rcc)
        if(TARGET Qt6::Widgets)
            qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS uic)
        endif()
    endif()
endforeach()
