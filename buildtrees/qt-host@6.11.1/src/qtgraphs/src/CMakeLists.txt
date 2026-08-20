# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake")

set(QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES "QT_COMMERCIAL_OR_GPL3")

if(NOT QT_FEATURE_graphs_2d AND NOT QT_FEATURE_graphs_3d)
    message("You must have either FEATURE_graphs_2d or FEATURE_graphs_3d ON, or nothing will be built.")
else()
    # 2D specific resources
    set(shader_resource_files_2D
        "graphs2d/data/gridshader.frag"
        "graphs2d/data/gridshader.vert"
        "graphs2d/data/tickershader.frag"
        "graphs2d/data/tickershader.vert"
        "graphs2d/data/tickershaderhorizontal.frag"
        "graphs2d/data/tickershaderhorizontal.vert"
        "graphs2d/data/lineshaderhorizontal.frag"
        "graphs2d/data/lineshaderhorizontal.vert"
        "graphs2d/data/lineshadervertical.frag"
        "graphs2d/data/lineshadervertical.vert"
    )

    set(qml_component_resources
        "graphs3d/qml/resources/DatapointSphere.qml"
        "graphs3d/qml/resources/DatapointCube.qml"
    )

    foreach(file IN LISTS qml_component_resources)
        get_filename_component(filename ${file} NAME)
        set(RESOURCE_NAME ${filename})
        string(REPLACE ".qml" "" RESOURCE_NAME ${RESOURCE_NAME})
        set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS ${RESOURCE_NAME})
    endforeach()

    set(qml_axis_resources
        "graphs3d/qml/resources/AxisLabel.qml"
        "graphs3d/qml/resources/ItemLabel.qml"
        "graphs3d/qml/resources/TitleLabel.qml"
    )

    foreach(file IN LISTS qml_axis_resources)
        get_filename_component(filename ${file} NAME)
        set(RESOURCE_NAME ${filename})
        string(REPLACE ".qml" "" RESOURCE_NAME ${RESOURCE_NAME})
        set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS ${RESOURCE_NAME})
    endforeach()

    set(qml_material_resources
        "graphs3d/qml/resources/ObjectGradientMaterial.qml"
        "graphs3d/qml/resources/ObjectGradientMaterialInstancing.qml"
        "graphs3d/qml/resources/BarsMaterial.qml"
        "graphs3d/qml/resources/BarsMaterialInstancing.qml"
        "graphs3d/qml/resources/SurfaceMaterial.qml"
        "graphs3d/qml/resources/ScatterMaterial.qml"
        "graphs3d/qml/resources/ScatterMaterialInstancing.qml"
        "graphs3d/qml/resources/SplineMaterial.qml"
        "graphs3d/qml/resources/GridSurfaceMaterial.qml"
        "graphs3d/qml/resources/SurfaceSliceMaterial.qml"
        "graphs3d/qml/resources/SurfaceShadowNoTex.qml"
        "graphs3d/qml/resources/VolumeMaterial.qml"
        "graphs3d/qml/resources/VolumeLowDefMaterial.qml"
        "graphs3d/qml/resources/VolumeSliceMaterial.qml"
        "graphs3d/qml/resources/VolumeFrameMaterial.qml"
        "graphs3d/qml/resources/BackgroundMaterial.qml"
    )

    foreach(file IN LISTS qml_material_resources)
        get_filename_component(filename ${file} NAME)
        set(RESOURCE_NAME ${filename})
        string(REPLACE ".qml" "" RESOURCE_NAME ${RESOURCE_NAME})
        set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS ${RESOURCE_NAME})
    endforeach()

    set(resources
        "graphs2d/qml/designer/Graphs.metainfo"
        "graphs2d/qml/designer/images/bars3d-icon.png"
        "graphs2d/qml/designer/images/bars3d-icon16.png"
        "graphs2d/qml/designer/images/scatter3d-icon.png"
        "graphs2d/qml/designer/images/scatter3d-icon16.png"
        "graphs2d/qml/designer/images/surface3d-icon.png"
        "graphs2d/qml/designer/images/surface3d-icon16.png"
    )

    set(mesh_resource_files
        "graphs3d/engine/meshes/backgroundMesh.mesh"
        "graphs3d/engine/meshes/backgroundNoFloorMesh.mesh"
        "graphs3d/engine/meshes/arrowMesh.mesh"
        "graphs3d/engine/meshes/arrowMeshSmooth.mesh"
        "graphs3d/engine/meshes/bevelBarMeshFull.mesh"
        "graphs3d/engine/meshes/bevelBarMeshSmoothFull.mesh"
        "graphs3d/engine/meshes/bevelBarMesh.mesh"
        "graphs3d/engine/meshes/bevelBarMeshSmooth.mesh"
        "graphs3d/engine/meshes/coneMeshFull.mesh"
        "graphs3d/engine/meshes/coneMeshSmoothFull.mesh"
        "graphs3d/engine/meshes/coneMesh.mesh"
        "graphs3d/engine/meshes/coneMeshSmooth.mesh"
        "graphs3d/engine/meshes/barMeshFull.mesh"
        "graphs3d/engine/meshes/barMeshSmoothFull.mesh"
        "graphs3d/engine/meshes/barMesh.mesh"
        "graphs3d/engine/meshes/barMeshSmooth.mesh"
        "graphs3d/engine/meshes/cylinderMeshFull.mesh"
        "graphs3d/engine/meshes/cylinderMeshSmoothFull.mesh"
        "graphs3d/engine/meshes/cylinderMesh.mesh"
        "graphs3d/engine/meshes/cylinderMeshSmooth.mesh"
        "graphs3d/engine/meshes/minimalMesh.mesh"
        "graphs3d/engine/meshes/minimalMeshSmooth.mesh"
        "graphs3d/engine/meshes/planeMesh.mesh"
        "graphs3d/engine/meshes/pyramidMeshFull.mesh"
        "graphs3d/engine/meshes/pyramidMeshSmoothFull.mesh"
        "graphs3d/engine/meshes/pyramidMesh.mesh"
        "graphs3d/engine/meshes/pyramidMeshSmooth.mesh"
        "graphs3d/engine/meshes/octagonMesh.mesh"
        "graphs3d/engine/meshes/sphereMesh.mesh"
        "graphs3d/engine/meshes/sphereMeshSmooth.mesh"
    )

    foreach(file IN LISTS mesh_resource_files)
        get_filename_component(filename ${file} NAME)
        set(RESOURCE_NAME ${filename})
        string(REPLACE ".mesh" "" RESOURCE_NAME ${RESOURCE_NAME})
        set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS ${RESOURCE_NAME})
    endforeach()

    set(shader_resource_files
        "graphs3d/engine/shaders/3dsliceframes.frag"
        "graphs3d/engine/shaders/position.vert"
        "graphs3d/engine/shaders/objectgradient.vert"
        "graphs3d/engine/shaders/objectgradient.frag"
        "graphs3d/engine/shaders/objectgradientinstancing.vert"
        "graphs3d/engine/shaders/objectgradientinstancing.frag"
        "graphs3d/engine/shaders/rangegradient.frag"
        "graphs3d/engine/shaders/rangegradientinstancing.frag"
        "graphs3d/engine/shaders/surface.vert"
        "graphs3d/engine/shaders/surface.frag"
        "graphs3d/engine/shaders/surfaceGrid.vert"
        "graphs3d/engine/shaders/surfaceGrid.frag"
        "graphs3d/engine/shaders/bars.frag"
        "graphs3d/engine/shaders/barsinstancing.vert"
        "graphs3d/engine/shaders/barsinstancing.frag"
        "graphs3d/engine/shaders/scatter.vert"
        "graphs3d/engine/shaders/scatter.frag"
        "graphs3d/engine/shaders/scatterinstancing.vert"
        "graphs3d/engine/shaders/scatterinstancing.frag"
        "graphs3d/engine/shaders/surfaceSlice.vert"
        "graphs3d/engine/shaders/spline.vert"
        "graphs3d/engine/shaders/spline.frag"
        "graphs3d/engine/shaders/texture3d.frag"
        "graphs3d/engine/shaders/texture3d.vert"
        "graphs3d/engine/shaders/texture3dlowdef.frag"
        "graphs3d/engine/shaders/texture3dslice.frag"
        "graphs3d/engine/shaders/backgroundgrid.frag"
    )

    foreach(file IN LISTS shader_resource_files)
        get_filename_component(filename ${file} NAME)
        set(RESOURCE_NAME ${filename})
        string(REPLACE "." "" RESOURCE_NAME ${RESOURCE_NAME})
        set_source_files_properties(${file} PROPERTIES QT_RESOURCE_ALIAS ${RESOURCE_NAME})
    endforeach()

    # First check if both 2D and 3D are enabled. If it is, we must add dependencies to
    # both (i.e. QtQuick3D, Quick3DPrivate, Quick3DRuntimeRenderPrivate,
    # QtQuickPrivate, and QtQuickShapesPrivate).
    if(QT_FEATURE_graphs_3d AND QT_FEATURE_graphs_2d)
        qt_internal_add_qml_module(Graphs
            URI "QtGraphs"
            TARGET_PRODUCT "Qt Graphs (Qt $$QT_VERSION)"
            TARGET_DESCRIPTION "Qt Graphs Visualization component for Qt."
            VERSION "${PROJECT_VERSION}"
            PAST_MAJOR_VERSIONS 1
            DEPENDENCIES
                QtQuick
                # 3D specific
                QtQuick3D
            LIBRARIES
                # Common
                Qt::CorePrivate
                Qt::QuickPrivate
                # 2D specific
                Qt::QuickShapesPrivate
                # 3D specific
                Qt::Quick3DPrivate
                Qt::Quick3DRuntimeRenderPrivate
            PUBLIC_LIBRARIES
                Qt::Core
                Qt::Gui
                Qt::Quick
                Qt::Quick3D
            PRIVATE_MODULE_INTERFACE
                # Common
                Qt::CorePrivate
                Qt::QuickPrivate
                # 3D specific
                Qt::Quick3DPrivate
                Qt::Quick3DRuntimeRenderPrivate
        )

        qt_internal_add_shaders(Graphs "graphsshaders2D"
            SILENT
            BATCHABLE
            PRECOMPILE
            OPTIMIZED
            MULTIVIEW
            PREFIX
                "/shaders"
            BASE
                "graphs2d/data"
            FILES
                ${shader_resource_files_2D}
        )

        qt_internal_add_resource(Graphs "graphsshaders"
            PREFIX
                "/shaders"
            BASE
                "graphs3d/engine/shaders"
            FILES
                ${shader_resource_files}
        )

        qt_internal_add_resource(Graphs "graphsmeshes"
            PREFIX
                "/defaultMeshes"
            BASE
                "graphs3d/engine/meshes"
            FILES
                ${mesh_resource_files}
                ${obj_resource_files}
        )

        qt_internal_add_resource(Graphs "qml_component_resources"
            PREFIX
                "/datapointModels"
            BASE
                "graphs3d/qml"
            FILES
                ${qml_component_resources}
        )

        qt_internal_add_resource(Graphs "qml_axis_resources"
            PREFIX
                "/axis"
            BASE
                "graphs3d/qml"
            FILES
                ${qml_axis_resources}
        )

        qt_internal_add_resource(Graphs "qml_material_resources"
            PREFIX
                "/materials"
            BASE
                "graphs3d/qml"
            FILES
                ${qml_material_resources}
        )
    else()
        if(QT_FEATURE_graphs_3d)
            qt_internal_add_qml_module(Graphs
                URI "QtGraphs"
                TARGET_PRODUCT "Qt Graphs (Qt $$QT_VERSION)"
                TARGET_DESCRIPTION "Qt Graphs Visualization component for Qt."
                VERSION "${PROJECT_VERSION}"
                PAST_MAJOR_VERSIONS 1
                DEPENDENCIES
                    QtQuick
                    QtQuick3D
                LIBRARIES
                    Qt::QuickPrivate
                    Qt::Quick3DPrivate
                    Qt::Quick3DRuntimeRenderPrivate
                PUBLIC_LIBRARIES
                    Qt::Core
                    Qt::Gui
                    Qt::Quick
                    Qt::Quick3D
                PRIVATE_MODULE_INTERFACE
                    Qt::QuickPrivate
                    Qt::Quick3DPrivate
                    Qt::Quick3DRuntimeRenderPrivate
            )

            # CMake 3.16 forces these to be added here instead of the graphs3d folder
            qt_internal_add_resource(Graphs "graphsshaders"
                PREFIX
                    "/shaders"
                BASE
                    "graphs3d/engine/shaders"
                FILES
                    ${shader_resource_files}
            )

            qt_internal_add_resource(Graphs "graphsmeshes"
                PREFIX
                    "/defaultMeshes"
                BASE
                    "graphs3d/engine/meshes"
                FILES
                    ${mesh_resource_files}
                    ${obj_resource_files}
            )

            qt_internal_add_resource(Graphs "qml_component_resources"
                PREFIX
                    "/datapointModels"
                BASE
                    "graphs3d/qml"
                FILES
                    ${qml_component_resources}
            )

            qt_internal_add_resource(Graphs "qml_axis_resources"
                PREFIX
                    "/axis"
                BASE
                    "graphs3d/qml"
                FILES
                    ${qml_axis_resources}
            )

            qt_internal_add_resource(Graphs "qml_material_resources"
                PREFIX
                    "/materials"
                BASE
                    "graphs3d/qml"
                FILES
                    ${qml_material_resources}
            )
        else(QT_FEATURE_graphs_2d)
            qt_internal_add_qml_module(Graphs
                URI "QtGraphs"
                TARGET_PRODUCT "Qt Graphs (Qt $$QT_VERSION)"
                TARGET_DESCRIPTION "Qt Graphs Visualization component for Qt."
                VERSION "${PROJECT_VERSION}"
                PAST_MAJOR_VERSIONS 1
                DEPENDENCIES
                    QtQuick
                LIBRARIES
                    Qt::QuickPrivate
                    Qt::QuickShapesPrivate
                PUBLIC_LIBRARIES
                    Qt::Core
                    Qt::Gui
                    Qt::Quick
            )

            qt_internal_add_shaders(Graphs "graphsshaders2D"
                SILENT
                BATCHABLE
                PRECOMPILE
                OPTIMIZED
                MULTIVIEW
                PREFIX
                    "/shaders"
                BASE
                    "graphs2d/data"
                FILES
                    ${shader_resource_files_2D}
            )
        endif()
    endif() # if(QT_FEATURE_graphs_3d AND QT_FEATURE_graphs_2d)
endif()

qt_configure_add_summary_section(NAME "Qt Graphs Support")
qt_configure_add_summary_entry(ARGS "graphs-2d")
qt_configure_add_summary_entry(ARGS "graphs-3d")
qt_configure_end_summary_section() # end of "Qt Graphs Support" section

set(TRACE_SOURCES "")
set(TRACE_SOURCES_2D "")

add_subdirectory(common)

qt_internal_extend_target(Graphs
    INCLUDE_DIRECTORIES
        common
)

if(QT_FEATURE_graphs_3d)
    add_subdirectory(graphs3d)
endif()

if(QT_FEATURE_graphs_2d)
    add_subdirectory(graphs2d)
endif()


qt_internal_generate_tracepoints(Graphs graphs
    SOURCES
        ${TRACE_SOURCES}
        ${TRACE_SOURCES_2D}
)

qt_internal_add_docs(Graphs
    doc/qtgraphs.qdocconf
)
