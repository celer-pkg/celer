# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtFindWrapHelper NO_POLICY_SCOPE)

qt_find_package_system_or_bundled(wrap_openxr
    FRIENDLY_PACKAGE_NAME "OpenXR"
    WRAP_PACKAGE_TARGET "WrapOpenXR::WrapOpenXR"
    WRAP_PACKAGE_FOUND_VAR_NAME "WrapOpenXR_FOUND"
    BUNDLED_PACKAGE_NAME "BundledOpenXR"
    BUNDLED_PACKAGE_TARGET "BundledOpenXR"
    SYSTEM_PACKAGE_NAME "WrapSystemOpenXR"
    SYSTEM_PACKAGE_TARGET "WrapSystemOpenXR::WrapSystemOpenXR"
)
