// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles

Emitter {
    anchors.fill: parent
    velocity: AngleDirection{angleVariation: 360; magnitude: 140; magnitudeVariation: 40}
    enabled: false;
    lifeSpan: 500;
    emitRate: 1
    size: 28
    endSize: 14
    group: "yellow"
}
