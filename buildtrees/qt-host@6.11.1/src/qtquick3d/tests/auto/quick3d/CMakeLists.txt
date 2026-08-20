# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from quick3d.pro.

add_subdirectory(qquick3dperspectivecamera)
add_subdirectory(qquick3dorthographiccamera)
add_subdirectory(qquick3dfrustumcamera)
add_subdirectory(qquick3dcustomcamera)
add_subdirectory(qquick3dtexture)
add_subdirectory(qquick3dtexturedata)
add_subdirectory(qquick3dloader)
add_subdirectory(qquick3dmaterials)
add_subdirectory(qquick3ddirectionallight)
add_subdirectory(qquick3dpointlight)
add_subdirectory(qquick3dspotlight)
add_subdirectory(qquick3dnode)
add_subdirectory(qquick3dmodel)
add_subdirectory(qquick3dgeometry)
add_subdirectory(qquick3dresourceloader)
add_subdirectory(qquick3dreflectionprobe)
if (TARGET Qt::QuickTestUtilsPrivate AND TARGET Qt::QmlModelsPrivate)
    add_subdirectory(qquick3drepeater)
endif()
add_subdirectory(qquick3dlightmapio)
