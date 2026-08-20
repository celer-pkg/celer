# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from quick3d.pro.

#####################################################################
## Quick3D Module:
#####################################################################

qt_internal_add_module(Quick3D
    SOURCES
        qquick3d.cpp qquick3d.h
        qquick3dabstractlight.cpp qquick3dabstractlight_p.h
        qquick3dcamera.cpp qquick3dcamera_p.h
        qquick3dcustomcamera.cpp qquick3dcustomcamera_p.h
        qquick3dcustommaterial.cpp qquick3dcustommaterial_p.h
        qquick3ddebugsettings_p.h qquick3ddebugsettings.cpp
        qquick3ddefaultmaterial.cpp qquick3ddefaultmaterial_p.h
        qquick3ddirectionallight.cpp qquick3ddirectionallight_p.h
        qquick3deffect.cpp qquick3deffect_p.h
        qquick3dfrustumcamera.cpp qquick3dfrustumcamera_p.h
        qquick3dgeometry.cpp qquick3dgeometry.h qquick3dgeometry_p.h
        qquick3dinstancing.cpp qquick3dinstancing.h qquick3dinstancing_p.h
        qquick3ditem2d.cpp qquick3ditem2d_p.h
        qquick3djoint.cpp qquick3djoint_p.h
        qquick3dloader.cpp qquick3dloader_p.h
        qquick3dmaterial.cpp qquick3dmaterial_p.h
        qquick3dmodel.cpp qquick3dmodel_p.h
        qquick3dnode.cpp qquick3dnode_p.h
        qquick3dnode_p_p.h
        qquick3dobject.cpp qquick3dobject.h qquick3dobject_p.h
        qquick3dobjectchangelistener_p.h
        qquick3dorthographiccamera.cpp qquick3dorthographiccamera_p.h
        qquick3dperspectivecamera.cpp qquick3dperspectivecamera_p.h
        qquick3dpickresult.cpp qquick3dpickresult_p.h
        qquick3dpointlight.cpp qquick3dpointlight_p.h
        qquick3dprincipledmaterial.cpp qquick3dprincipledmaterial_p.h
        qquick3dquaternionanimation.cpp qquick3dquaternionanimation_p.h
        qquick3dquaternionutils.cpp qquick3dquaternionutils_p.h
        qquick3dreflectionprobe.cpp qquick3dreflectionprobe_p.h
        qquick3drenderpass.cpp qquick3drenderpass_p.h
        qquick3drenderstats.cpp qquick3drenderstats_p.h
        qquick3drepeater.cpp qquick3drepeater_p.h
        qquick3dresourceloader.cpp qquick3dresourceloader_p.h
        qquick3dsceneenvironment.cpp qquick3dsceneenvironment_p.h
        qquick3dscenemanager.cpp qquick3dscenemanager_p.h
        qquick3dscenerenderer.cpp qquick3dscenerenderer_p.h
        qquick3dscenerootnode.cpp qquick3dscenerootnode_p.h
        qquick3dshaderutils.cpp qquick3dshaderutils_p.h
        qquick3dskeleton.cpp qquick3dskeleton_p.h
        qquick3dspotlight.cpp qquick3dspotlight_p.h
        qquick3dmorphtarget.cpp qquick3dmorphtarget_p.h
        qquick3dtexture.cpp qquick3dtexture_p.h
        qquick3dtexturedata.cpp qquick3dtexturedata.h qquick3dtexturedata_p.h
        qquick3dskin.cpp qquick3dskin_p.h
        qquick3dutils_p.h
        qquick3dviewport.cpp qquick3dviewport_p.h
        qtquick3dglobal.h qtquick3dglobal_p.h
        qquick3dcubemaptexture.cpp qquick3dcubemaptexture_p.h
        qquick3dbakedlightmap.cpp qquick3dbakedlightmap_p.h
        qquick3dlightmapper.cpp qquick3dlightmapper_p.h
        qquick3dspecularglossymaterial.cpp qquick3dspecularglossymaterial_p.h
        qquick3dfog.cpp qquick3dfog_p.h
        qquick3dlightmapbaker.cpp qquick3dlightmapbaker_p.h
        qquick3dcontentlayer.cpp qquick3dcontentlayer_p.h
        extensions/qquick3drenderextensions.cpp extensions/qquick3drenderextensions.h
        extensions/qquick3dextensionhelpers.cpp extensions/qquick3dextensionhelpers.h
        extensions/qquick3dtextureproviderextension.cpp extensions/qquick3dtextureproviderextension.h extensions/qquick3dtextureproviderextension_p.h
    SSG_HEADER_FILTERS
        "(^|/)qquick3dextensionhelpers\\.h$"
    DEFINES
        QT_BUILD_QUICK3D_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3DRuntimeRender
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::QuickPrivate
)

qt_internal_add_qml_module(Quick3D
    URI "QtQuick3D"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    DEPENDENCIES QtQuick/auto
    CLASS_NAME QQuick3DPlugin
    PLUGIN_TARGET qquick3dplugin
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    QML_FILES LightmapperOutputWindow.qml
)

qt_internal_extend_target(qquick3dplugin
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::Quick3DPrivate
)

qt_internal_generate_tracepoints(Quick3D quick3d
    SOURCES
        qquick3dscenerenderer.cpp
)

set_property(SOURCE qquick3dgeometry.cpp PROPERTY SKIP_AUTOMOC ON) # special case
set_property(SOURCE qquick3dtexturedata.cpp PROPERTY SKIP_AUTOMOC ON) # special case

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()

qt_internal_add_docs(Quick3D
    doc/qtquick3d.qdocconf
)
