# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(target_name qqem)

qt_internal_add_app(${target_name}
    SOURCES
        main.cpp
        addnodemodel.cpp addnodemodel.h
        applicationsettings.cpp applicationsettings.h
        arrowsmodel.cpp arrowsmodel.h
        effectmanager.cpp effectmanager.h
        fpshelper.cpp fpshelper.h
        nodesmodel.cpp nodesmodel.h
        nodeview.cpp nodeview.h
        propertyhandler.cpp propertyhandler.h
        qsbinspectorhelper.cpp qsbinspectorhelper.h
        shaderfeatures.cpp shaderfeatures.h
        syntaxhighlighter.cpp syntaxhighlighter.h
        syntaxhighlighterdata.cpp syntaxhighlighterdata.h
        uniformmodel.cpp uniformmodel.h
        codehelper.cpp codehelper.h
        codecompletionmodel.cpp codecompletionmodel.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::QuickPrivate
        Qt::ShaderToolsPrivate
)

# Resources:
set(qml_resource_files
    "../../nodes/common/BlurHelper.qml"
    "../../nodes/common/bluritems.frag"
    "../../nodes/common/bluritems.frag.qsb"
    "../../nodes/common/bluritems.vert"
    "../../nodes/common/bluritems.vert.qsb"
    "qml/AboutDialog.qml"
    "qml/AddNodeDialog.qml"
    "qml/ApplicationSettingsDialog.qml"
    "qml/ClearNodeViewDialog.qml"
    "qml/CodeEditor.qml"
    "qml/CustomDialog.qml"
    "qml/CustomIconButton.qml"
    "qml/CustomImageSelector.qml"
    "qml/CustomModeToggle.qml"
    "qml/CustomPopup.qml"
    "qml/CustomSplitView.qml"
    "qml/CustomTextEdit.qml"
    "qml/CustomTextField.qml"
    "qml/DeleteNodeDialog.qml"
    "qml/EditorView.qml"
    "qml/EffectPreview.qml"
    "qml/EffectPreviewToolbar.qml"
    "qml/ExportEffectDialog.qml"
    "qml/FindBar.qml"
    "qml/HelpView.qml"
    "qml/MainToolbar.qml"
    "qml/MainView.qml"
    "qml/NewProjectDialog.qml"
    "qml/NodeView.qml"
    "qml/OutputView.qml"
    "qml/PlaybackTimeComponent.qml"
    "qml/ProjectSettingsDialog.qml"
    "qml/PropertiesView.qml"
    "qml/PropertyEditDialog.qml"
    "qml/QsbInspectorDialog.qml"
    "qml/RenameNodeDialog.qml"
    "qml/SaveProjectDialog.qml"
    "qml/StatusBar.qml"
    "qml/about_effect/AboutEffect1.qml"
    "qml/about_effect/abouteffect1.frag.qsb"
    "qml/about_effect/abouteffect1.vert.qsb"
    "qml/images/background_colorful.jpg"
    "qml/images/background_dark.jpg"
    "qml/images/background_light.jpg"
    "qml/images/button_qsb.png"
    "qml/images/button_show.png"
    "qml/images/icon_add.png"
    "qml/images/icon_api.png"
    "qml/images/icon_api_on.png"
    "qml/images/icon_arrow.png"
    "qml/images/icon_arrow_down.png"
    "qml/images/icon_arrow_up.png"
    "qml/images/icon_autoplay.png"
    "qml/images/icon_autoplay_on.png"
    "qml/images/icon_borders.png"
    "qml/images/icon_borders_on.png"
    "qml/images/icon_distribute_nodes.png"
    "qml/images/icon_edit.png"
    "qml/images/icon_error.png"
    "qml/images/icon_layout_nodes.png"
    "qml/images/icon_pause.png"
    "qml/images/icon_play.png"
    "qml/images/icon_requires.png"
    "qml/images/icon_remove_shadow.png"
    "qml/images/icon_reset.png"
    "qml/images/icon_restart.png"
    "qml/images/icon_settings.png"
    "qml/images/icon_settings_on.png"
    "qml/images/icon_soften.png"
    "qml/images/icon_soften_on.png"
    "qml/images/icon_viewseparator.png"
    "qml/images/icon_visibility_off.png"
    "qml/images/icon_visibility_on.png"
    "qml/images/icon_zoom_auto.png"
    "qml/images/icon_zoom_in.png"
    "qml/images/icon_zoom_out.png"
    "qml/images/icon_export.png"
    "qml/images/item_border.png"
    "qml/images/nodeview_background.jpg"
    "qml/images/toggle_background.png"
    "qml/images/toggle_m1.png"
    "qml/images/toggle_m2.png"
    "qml/main.qml"
    "qqem_help.html"
    "qtquickcontrols2.conf"
    "qml/fonts/SourceCodePro-Bold.ttf"
    "qml/fonts/SourceCodePro-It.ttf"
    "qml/fonts/SourceCodePro-Regular.ttf"
    "qqem.ico"
)

qt_internal_add_resource(${target_name} "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

# See if optional syntax highlighting dependency exists
if (Qt6Quick3DGlslParserPrivate_FOUND)
    target_compile_definitions(${target_name} PRIVATE
        QQEM_SYNTAX_HIGHLIGHTING_ENABLED
    )
    target_link_libraries(${target_name} PRIVATE
        Qt::Quick3DGlslParserPrivate
    )
else ()
    message(NOTICE "Note: No Quick3DGlslParserPrivate available, building without syntax highlighting")
endif ()

# Application icon
if(WIN32)
    set_target_properties(${target_name} PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/qqem.ico"
    )
endif()

if(APPLE)
    set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "qqem.icns"
        OUTPUT_NAME "qqem"
    )
    set_source_files_properties(qqem.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(${target_name} PRIVATE
        qqem.icns
    )
    # special case end
    # Set values to be replaced in the custom Info_mac.plist.
    set(ICON "qqem.icns")
    set(EXECUTABLE "qqem")
    # special case end
endif()
