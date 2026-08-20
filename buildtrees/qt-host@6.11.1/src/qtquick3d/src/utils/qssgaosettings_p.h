// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGAOSETTINGS_H
#define QSSGAOSETTINGS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qtypes.h>
#include <QtQuick3DUtils/qtquick3dutilsexports.h>

QT_BEGIN_NAMESPACE

struct QSSGAmbientOcclusionSettings
{
    float aoStrength = 0.0f;
    float aoDistance = 5.0f;
    float aoSoftness = 50.0f;
    float aoBias = 0.0f;
    qint32 aoSamplerate = 2;
    bool aoDither = false;
};

QT_END_NAMESPACE

#endif // QSSGAOSETTINGS_H
