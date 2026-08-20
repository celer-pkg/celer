# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_internal_add_example(antialiasing)
qt_internal_add_example(customshaders)
qt_internal_add_example(hellocube)
qt_internal_add_example(intro)
qt_internal_add_example(view3d)
qt_internal_add_example(picking)
qt_internal_add_example(lights)
qt_internal_add_example(custommaterial)
qt_internal_add_example(principledmaterial)
qt_internal_add_example(proceduraltexture)
qt_internal_add_example(dynamiccreation)
qt_internal_add_example(quickitems)
qt_internal_add_example(customgeometry)
qt_internal_add_example(customeffect)
qt_internal_add_example(custominstancing)
qt_internal_add_example(instancing)
qt_internal_add_example(skinning)
qt_internal_add_example(offlineshaders)
qt_internal_add_example(submeshes)
qt_internal_add_example(quickball)
qt_internal_add_example(helloqtquick3d)
qt_internal_add_example(morphing)
qt_internal_add_example(custommorphing)
qt_internal_add_example(particles3d)
qt_internal_add_example(runtimeloader)
qt_internal_add_example(screenspacereflections)
qt_internal_add_example(reflectionprobes)
qt_internal_add_example(bakedlightmap)
qt_internal_add_example(lodhelper)
qt_internal_add_example(simplefog)
qt_internal_add_example(sceneeffects)
qt_internal_add_example(volumeraycaster)
qt_internal_add_example(extensions/stenciloutline3d)
qt_internal_add_example(cascadedshadowmaps)
qt_internal_add_example(orderindependenttransparency)
qt_internal_add_example(layers)
qt_internal_add_example(userpasses)

if(QT_ALLOW_DOWNLOAD)
    qt_internal_add_example(ssgilightmap)
else()
    message(DEBUG "Skipping ssgilightmap example because it is built as part of Qt "
        "and QT_ALLOW_DOWNLOAD is OFF.")
endif()

if(QT_FEATURE_quick3dxr_openxr)
    qt_internal_add_example(xr_simple)
    qt_internal_add_example(xr_anchors)
    qt_internal_add_example(xr_freeformteleportation)
    qt_internal_add_example(xr_input)
    qt_internal_add_example(xr_touch)
    qt_internal_add_example(xr_interaction)
endif()

# qt_examples_build_end() misses at least some of these due to some
# source subdirectories being added multiple times. See QTBUG-96159.
if(QT_FEATURE_quick3dxr_openxr)
    set(reused_dir_targets
        xr_interaction_shared
        xr_interaction_shared_init
        xr_touch_shared
        xr_touch_shared_init
    )
endif()

foreach(target IN LISTS reused_dir_targets)
    if(TARGET ${target})
        qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS moc rcc)
        if(TARGET Qt6::Widgets)
            qt_autogen_tools(${target} ENABLE_AUTOGEN_TOOLS uic)
        endif()
    endif()
endforeach()
