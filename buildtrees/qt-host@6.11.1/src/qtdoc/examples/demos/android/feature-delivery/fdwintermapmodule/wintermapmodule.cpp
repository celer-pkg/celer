// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QObject>
#include <QDebug>

extern "C" Q_DECL_EXPORT void* loadMapInfo()
{
    return new QString("Maps generated with AI");
}
