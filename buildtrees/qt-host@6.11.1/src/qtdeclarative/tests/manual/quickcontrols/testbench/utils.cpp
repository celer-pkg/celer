// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "utils.h"

#include <QDir>
#include <QtQuickControls2/private/qquickstyle_p.h>

Utils::Utils(QObject *parent)
    : QObject(parent)
    , mIconUrl(QUrl::fromLocalFile(QDir(APP_SOURCE_DIR + QStringLiteral(
        "/../../../auto/quickcontrols/qquickiconlabel/data/heart.svg")).canonicalPath()))
{
}

QStringList Utils::availableStyles() const
{
    return QQuickStylePrivate::builtInStyles();
}

QUrl Utils::iconUrl() const
{
    return mIconUrl;
}
