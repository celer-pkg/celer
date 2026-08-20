# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(DataVisualization
    TARGET_PRODUCT "Qt Data Visualization (Qt $$QT_VERSION)"
    TARGET_DESCRIPTION "3D Data Visualization component for Qt."
    SOURCES
        axis/qabstract3daxis.cpp axis/qabstract3daxis.h axis/qabstract3daxis_p.h
        axis/qcategory3daxis.cpp axis/qcategory3daxis.h axis/qcategory3daxis_p.h
        axis/qlogvalue3daxisformatter.cpp axis/qlogvalue3daxisformatter.h axis/qlogvalue3daxisformatter_p.h
        axis/qvalue3daxis.cpp axis/qvalue3daxis.h axis/qvalue3daxis_p.h
        axis/qvalue3daxisformatter.cpp axis/qvalue3daxisformatter.h axis/qvalue3daxisformatter_p.h
        data/abstractitemmodelhandler.cpp data/abstractitemmodelhandler_p.h
        data/abstractrenderitem.cpp data/abstractrenderitem_p.h
        data/baritemmodelhandler.cpp data/baritemmodelhandler_p.h
        data/barrenderitem.cpp data/barrenderitem_p.h
        data/customrenderitem.cpp data/customrenderitem_p.h
        data/labelitem.cpp data/labelitem_p.h
        data/qabstract3dseries.cpp data/qabstract3dseries.h data/qabstract3dseries_p.h
        data/qabstractdataproxy.cpp data/qabstractdataproxy.h data/qabstractdataproxy_p.h
        data/qbar3dseries.cpp data/qbar3dseries.h data/qbar3dseries_p.h
        data/qbardataitem.cpp data/qbardataitem.h data/qbardataitem_p.h
        data/qbardataproxy.cpp data/qbardataproxy.h data/qbardataproxy_p.h
        data/qcustom3ditem.cpp data/qcustom3ditem.h data/qcustom3ditem_p.h
        data/qcustom3dlabel.cpp data/qcustom3dlabel.h data/qcustom3dlabel_p.h
        data/qcustom3dvolume.cpp data/qcustom3dvolume.h data/qcustom3dvolume_p.h
        data/qheightmapsurfacedataproxy.cpp data/qheightmapsurfacedataproxy.h data/qheightmapsurfacedataproxy_p.h
        data/qitemmodelbardataproxy.cpp data/qitemmodelbardataproxy.h data/qitemmodelbardataproxy_p.h
        data/qitemmodelscatterdataproxy.cpp data/qitemmodelscatterdataproxy.h data/qitemmodelscatterdataproxy_p.h
        data/qitemmodelsurfacedataproxy.cpp data/qitemmodelsurfacedataproxy.h data/qitemmodelsurfacedataproxy_p.h
        data/qscatter3dseries.cpp data/qscatter3dseries.h data/qscatter3dseries_p.h
        data/qscatterdataitem.cpp data/qscatterdataitem.h data/qscatterdataitem_p.h
        data/qscatterdataproxy.cpp data/qscatterdataproxy.h data/qscatterdataproxy_p.h
        data/qsurface3dseries.cpp data/qsurface3dseries.h data/qsurface3dseries_p.h
        data/qsurfacedataitem.cpp data/qsurfacedataitem.h data/qsurfacedataitem_p.h
        data/qsurfacedataproxy.cpp data/qsurfacedataproxy.h data/qsurfacedataproxy_p.h
        data/scatteritemmodelhandler.cpp data/scatteritemmodelhandler_p.h
        data/scatterrenderitem.cpp data/scatterrenderitem_p.h
        data/surfaceitemmodelhandler.cpp data/surfaceitemmodelhandler_p.h
        engine/abstractdeclarativeinterface.cpp engine/abstractdeclarativeinterface_p.h
        engine/abstract3dcontroller.cpp engine/abstract3dcontroller_p.h
        engine/abstract3drenderer.cpp engine/abstract3drenderer_p.h
        engine/axisrendercache.cpp engine/axisrendercache_p.h
        engine/bars3dcontroller.cpp engine/bars3dcontroller_p.h
        engine/bars3drenderer.cpp engine/bars3drenderer_p.h
        engine/barseriesrendercache.cpp engine/barseriesrendercache_p.h
        engine/drawer.cpp engine/drawer_p.h
        engine/q3dbars.cpp engine/q3dbars.h engine/q3dbars_p.h
        engine/q3dcamera.cpp engine/q3dcamera.h engine/q3dcamera_p.h
        engine/q3dlight.cpp engine/q3dlight.h engine/q3dlight_p.h
        engine/q3dobject.cpp engine/q3dobject.h engine/q3dobject_p.h
        engine/q3dscatter.cpp engine/q3dscatter.h engine/q3dscatter_p.h
        engine/q3dscene.cpp engine/q3dscene.h engine/q3dscene_p.h
        engine/q3dsurface.cpp engine/q3dsurface.h engine/q3dsurface_p.h
        engine/qabstract3dgraph.cpp engine/qabstract3dgraph.h engine/qabstract3dgraph_p.h
        engine/scatter3dcontroller.cpp engine/scatter3dcontroller_p.h
        engine/scatter3drenderer.cpp engine/scatter3drenderer_p.h
        engine/scatterseriesrendercache.cpp engine/scatterseriesrendercache_p.h
        engine/selectionpointer.cpp engine/selectionpointer_p.h
        engine/seriesrendercache.cpp engine/seriesrendercache_p.h
        engine/surface3dcontroller.cpp engine/surface3dcontroller_p.h
        engine/surface3drenderer.cpp engine/surface3drenderer_p.h
        engine/surfaceseriesrendercache.cpp engine/surfaceseriesrendercache_p.h
        global/datavisualizationglobal_p.h
        global/qdatavisualizationglobal.h
        input/q3dinputhandler.cpp input/q3dinputhandler.h input/q3dinputhandler_p.h
        input/qabstract3dinputhandler.cpp input/qabstract3dinputhandler.h input/qabstract3dinputhandler_p.h
        input/qtouch3dinputhandler.cpp input/qtouch3dinputhandler.h input/qtouch3dinputhandler_p.h
        theme/q3dtheme.cpp theme/q3dtheme.h theme/q3dtheme_p.h
        theme/thememanager.cpp theme/thememanager_p.h
        utils/abstractobjecthelper.cpp utils/abstractobjecthelper_p.h
        utils/camerahelper.cpp utils/camerahelper_p.h
        utils/meshloader.cpp utils/meshloader_p.h
        utils/objecthelper.cpp utils/objecthelper_p.h
        utils/qutils.h
        utils/scatterobjectbufferhelper.cpp utils/scatterobjectbufferhelper_p.h
        utils/scatterpointbufferhelper.cpp utils/scatterpointbufferhelper_p.h
        utils/shaderhelper.cpp utils/shaderhelper_p.h
        utils/surfaceobject.cpp utils/surfaceobject_p.h
        utils/texturehelper.cpp utils/texturehelper_p.h
        utils/utils.cpp utils/utils_p.h
        utils/vertexindexer.cpp utils/vertexindexer_p.h
    NO_PCH_SOURCES
        # undef QT_NO_FOREACH:
        engine/abstract3dcontroller.cpp
        engine/abstract3drenderer.cpp
        engine/axisrendercache.cpp
        engine/bars3dcontroller.cpp
        engine/bars3drenderer.cpp
        engine/q3dscatter.cpp
        engine/q3dsurface.cpp
        engine/scatter3dcontroller.cpp
        engine/scatter3drenderer.cpp
        engine/surface3dcontroller.cpp
        engine/surface3drenderer.cpp
        # end undef QT_NO_FOREACH
    INCLUDE_DIRECTORIES
        axis
        data
        engine
        global
        input
        theme
        utils
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::OpenGL
)

set_source_files_properties("engine/meshes/arrowFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "arrow"
)
set_source_files_properties("engine/meshes/arrowSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "arrowSmooth"
)
set_source_files_properties("engine/meshes/background.obj"
    PROPERTIES QT_RESOURCE_ALIAS "background"
)
set_source_files_properties("engine/meshes/backgroundNoFloor.obj"
    PROPERTIES QT_RESOURCE_ALIAS "backgroundNoFloor"
)
set_source_files_properties("engine/meshes/barFilledFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "bevelbarFull"
)
set_source_files_properties("engine/meshes/barFilledSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "bevelbarSmoothFull"
)
set_source_files_properties("engine/meshes/barFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "bevelbar"
)
set_source_files_properties("engine/meshes/barSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "bevelbarSmooth"
)
set_source_files_properties("engine/meshes/coneFilledFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "coneFull"
)
set_source_files_properties("engine/meshes/coneFilledSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "coneSmoothFull"
)
set_source_files_properties("engine/meshes/coneFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "cone"
)
set_source_files_properties("engine/meshes/coneSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "coneSmooth"
)
set_source_files_properties("engine/meshes/cubeFilledFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "barFull"
)
set_source_files_properties("engine/meshes/cubeFilledSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "barSmoothFull"
)
set_source_files_properties("engine/meshes/cubeFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "bar"
)
set_source_files_properties("engine/meshes/cubeSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "barSmooth"
)
set_source_files_properties("engine/meshes/cylinderFilledFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "cylinderFull"
)
set_source_files_properties("engine/meshes/cylinderFilledSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "cylinderSmoothFull"
)
set_source_files_properties("engine/meshes/cylinderFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "cylinder"
)
set_source_files_properties("engine/meshes/cylinderSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "cylinderSmooth"
)
set_source_files_properties("engine/meshes/minimalFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "minimal"
)
set_source_files_properties("engine/meshes/minimalSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "minimalSmooth"
)
set_source_files_properties("engine/meshes/plane.obj"
    PROPERTIES QT_RESOURCE_ALIAS "plane"
)
set_source_files_properties("engine/meshes/pyramidFilledFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "pyramidFull"
)
set_source_files_properties("engine/meshes/pyramidFilledSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "pyramidSmoothFull"
)
set_source_files_properties("engine/meshes/pyramidFlat.obj"
    PROPERTIES QT_RESOURCE_ALIAS "pyramid"
)
set_source_files_properties("engine/meshes/pyramidSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "pyramidSmooth"
)
set_source_files_properties("engine/meshes/sphere.obj"
    PROPERTIES QT_RESOURCE_ALIAS "sphere"
)
set_source_files_properties("engine/meshes/sphereSmooth.obj"
    PROPERTIES QT_RESOURCE_ALIAS "sphereSmooth"
)
set(mesh_resource_files
    "engine/meshes/arrowFlat.obj"
    "engine/meshes/arrowSmooth.obj"
    "engine/meshes/background.obj"
    "engine/meshes/backgroundNoFloor.obj"
    "engine/meshes/barFilledFlat.obj"
    "engine/meshes/barFilledSmooth.obj"
    "engine/meshes/barFlat.obj"
    "engine/meshes/barSmooth.obj"
    "engine/meshes/coneFilledFlat.obj"
    "engine/meshes/coneFilledSmooth.obj"
    "engine/meshes/coneFlat.obj"
    "engine/meshes/coneSmooth.obj"
    "engine/meshes/cubeFilledFlat.obj"
    "engine/meshes/cubeFilledSmooth.obj"
    "engine/meshes/cubeFlat.obj"
    "engine/meshes/cubeSmooth.obj"
    "engine/meshes/cylinderFilledFlat.obj"
    "engine/meshes/cylinderFilledSmooth.obj"
    "engine/meshes/cylinderFlat.obj"
    "engine/meshes/cylinderSmooth.obj"
    "engine/meshes/minimalFlat.obj"
    "engine/meshes/minimalSmooth.obj"
    "engine/meshes/plane.obj"
    "engine/meshes/pyramidFilledFlat.obj"
    "engine/meshes/pyramidFilledSmooth.obj"
    "engine/meshes/pyramidFlat.obj"
    "engine/meshes/pyramidSmooth.obj"
    "engine/meshes/sphere.obj"
    "engine/meshes/sphereSmooth.obj"
)

set_source_files_properties("engine/shaders/3dsliceframes.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragment3DSliceFrames"
)
set_source_files_properties("engine/shaders/colorOnY.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentColorOnY"
)
set_source_files_properties("engine/shaders/colorOnY_ES2.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentColorOnYES2"
)
set_source_files_properties("engine/shaders/default.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragment"
)
set_source_files_properties("engine/shaders/default.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertex"
)
set_source_files_properties("engine/shaders/defaultNoMatrices.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexNoMatrices"
)
set_source_files_properties("engine/shaders/default_ES2.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentES2"
)
set_source_files_properties("engine/shaders/depth.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentDepth"
)
set_source_files_properties("engine/shaders/depth.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexDepth"
)
set_source_files_properties("engine/shaders/label.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentLabel"
)
set_source_files_properties("engine/shaders/label.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexLabel"
)
set_source_files_properties("engine/shaders/plainColor.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentPlainColor"
)
set_source_files_properties("engine/shaders/plainColor.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexPlainColor"
)
set_source_files_properties("engine/shaders/point_ES2.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexPointES2"
)
set_source_files_properties("engine/shaders/point_ES2_UV.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexPointES2_UV"
)
set_source_files_properties("engine/shaders/position.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexPosition"
)
set_source_files_properties("engine/shaders/positionmap.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentPositionMap"
)
set_source_files_properties("engine/shaders/shadow.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentShadow"
)
set_source_files_properties("engine/shaders/shadow.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexShadow"
)
set_source_files_properties("engine/shaders/shadowNoMatrices.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexShadowNoMatrices"
)
set_source_files_properties("engine/shaders/shadowNoTex.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentShadowNoTex"
)
set_source_files_properties("engine/shaders/shadowNoTexColorOnY.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentShadowNoTexColorOnY"
)
set_source_files_properties("engine/shaders/surface.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurface"
)
set_source_files_properties("engine/shaders/surfaceFlat.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurfaceFlat"
)
set_source_files_properties("engine/shaders/surfaceFlat.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexSurfaceFlat"
)
set_source_files_properties("engine/shaders/surfaceShadowFlat.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurfaceShadowFlat"
)
set_source_files_properties("engine/shaders/surfaceShadowFlat.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexSurfaceShadowFlat"
)
set_source_files_properties("engine/shaders/surfaceShadowNoTex.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurfaceShadowNoTex"
)
set_source_files_properties("engine/shaders/surfaceTexturedFlat.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurfaceTexturedFlat"
)
set_source_files_properties("engine/shaders/surfaceTexturedShadow.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexturedSurfaceShadow"
)
set_source_files_properties("engine/shaders/surfaceTexturedShadowFlat.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexturedSurfaceShadowFlat"
)
set_source_files_properties("engine/shaders/surface_ES2.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentSurfaceES2"
)
set_source_files_properties("engine/shaders/texture.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexture"
)
set_source_files_properties("engine/shaders/texture.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexTexture"
)
set_source_files_properties("engine/shaders/texture3d.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexture3D"
)
set_source_files_properties("engine/shaders/texture3d.vert"
    PROPERTIES QT_RESOURCE_ALIAS "vertexTexture3D"
)
set_source_files_properties("engine/shaders/texture3dlowdef.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexture3DLowDef"
)
set_source_files_properties("engine/shaders/texture3dslice.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTexture3DSlice"
)
set_source_files_properties("engine/shaders/texture_ES2.frag"
    PROPERTIES QT_RESOURCE_ALIAS "fragmentTextureES2"
)
set(shader_resource_files
    "engine/shaders/3dsliceframes.frag"
    "engine/shaders/colorOnY.frag"
    "engine/shaders/colorOnY_ES2.frag"
    "engine/shaders/default.frag"
    "engine/shaders/default.vert"
    "engine/shaders/defaultNoMatrices.vert"
    "engine/shaders/default_ES2.frag"
    "engine/shaders/depth.frag"
    "engine/shaders/depth.vert"
    "engine/shaders/label.frag"
    "engine/shaders/label.vert"
    "engine/shaders/plainColor.frag"
    "engine/shaders/plainColor.vert"
    "engine/shaders/point_ES2.vert"
    "engine/shaders/point_ES2_UV.vert"
    "engine/shaders/position.vert"
    "engine/shaders/positionmap.frag"
    "engine/shaders/shadow.frag"
    "engine/shaders/shadow.vert"
    "engine/shaders/shadowNoMatrices.vert"
    "engine/shaders/shadowNoTex.frag"
    "engine/shaders/shadowNoTexColorOnY.frag"
    "engine/shaders/surface.frag"
    "engine/shaders/surfaceFlat.frag"
    "engine/shaders/surfaceFlat.vert"
    "engine/shaders/surfaceShadowFlat.frag"
    "engine/shaders/surfaceShadowFlat.vert"
    "engine/shaders/surfaceShadowNoTex.frag"
    "engine/shaders/surfaceTexturedFlat.frag"
    "engine/shaders/surfaceTexturedShadow.frag"
    "engine/shaders/surfaceTexturedShadowFlat.frag"
    "engine/shaders/surface_ES2.frag"
    "engine/shaders/texture.frag"
    "engine/shaders/texture.vert"
    "engine/shaders/texture3d.frag"
    "engine/shaders/texture3d.vert"
    "engine/shaders/texture3dlowdef.frag"
    "engine/shaders/texture3dslice.frag"
    "engine/shaders/texture_ES2.frag"
)

qt_internal_add_resource(DataVisualization "datavisualizationshaders"
    PREFIX
        "/shaders"
    BASE
        "engine/shaders"
    FILES
        ${shader_resource_files}
)

qt_internal_add_resource(DataVisualization "datavisualizationmeshes"
    PREFIX
        "/defaultMeshes"
    BASE
        "engine/meshes"
    FILES
        ${mesh_resource_files}
)

qt_internal_extend_target(DataVisualization CONDITION MACOS
    LIBRARIES
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::GuiPrivate
)

qt_internal_extend_target(DataVisualization CONDITION MINGW
    NO_UNITY_BUILD_SOURCES
        data/qbardataitem.cpp
        data/qscatterdataitem.cpp
        data/qsurfacedataitem.cpp
    # MinGW triggers `-Werror=maybe-uninitialized` on d_ptr while
    # proper checks are in place and d_ptr is initialized in
    # operator=()
)

qt_internal_add_docs(DataVisualization
    doc/qtdatavis3d.qdocconf
)

