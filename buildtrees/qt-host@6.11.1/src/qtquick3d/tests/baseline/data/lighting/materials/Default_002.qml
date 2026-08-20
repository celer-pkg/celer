// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D

DefaultMaterial {
    id: default_002
    lighting: DefaultMaterial.FragmentLighting
    indexOfRefraction: 1.5
    specularAmount: 0
    specularRoughness: 0
    bumpAmount: 0.5
    translucentFalloff: 1
}
