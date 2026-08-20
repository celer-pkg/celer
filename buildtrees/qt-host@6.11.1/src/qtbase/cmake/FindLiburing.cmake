# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig QUIET)

pkg_check_modules(Liburing IMPORTED_TARGET "liburing")

if (NOT TARGET PkgConfig::Liburing)
    set(Liburing_FOUND 0)
endif()
