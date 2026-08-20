// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LIGHTMAPVIEWERHELPERS_H
#define LIGHTMAPVIEWERHELPERS_H

#include <QByteArray>
#include <QString>

#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>

struct LightmapViewerHelpers
{
    static void maskToBBGRColor(QByteArray &array, bool useAlpha = true);
    static bool processLightmap(const QString &filename, bool print, bool extract);
    static QString lightmapTagToString(QSSGLightmapIODataTag tag);
    static QSSGLightmapIODataTag stringToLightmapTag(const QString &tag);

    struct SceneMetadata
    {
        QString qtVersion;
        QString bakeStartTime;
        QString bakeDuration;
        QVariantList options;
    };
    static SceneMetadata processSceneMetadata(const QVariantMap &map);
    static QVariantList processMetadata(const QString &key, const QVariantMap &map);
};

#endif // LIGHTMAPVIEWERHELPERS_H
