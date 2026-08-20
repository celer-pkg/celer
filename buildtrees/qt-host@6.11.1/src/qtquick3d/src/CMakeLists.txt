# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# We need to include the features of a few modules before they are actually declared.
# The feature values are used as conditions for deciding whether bundled 3rd party libraries
# should be built.
# The order of evaluation matters.
qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/xr/configure.cmake")

set(QT_SBOM_DEFAULT_QT_LICENSE_ID_LIBRARIES "QT_COMMERCIAL_OR_GPL3")

add_subdirectory(3rdparty)
add_subdirectory(utils)
add_subdirectory(assetimport)
add_subdirectory(runtimerender)
add_subdirectory(quick3d)
add_subdirectory(quick3dparticles)
add_subdirectory(quick3dparticleeffects)
add_subdirectory(assetutils)
add_subdirectory(plugins)
add_subdirectory(effects)
add_subdirectory(helpers)
add_subdirectory(iblbaker)
add_subdirectory(glslparser)
add_subdirectory(xr)
