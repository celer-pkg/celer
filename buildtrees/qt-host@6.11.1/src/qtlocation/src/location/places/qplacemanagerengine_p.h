// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEMANAGERENGINE_P_H
#define QPLACEMANAGERENGINE_P_H

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

#include <QtCore/QString>
#include <QtLocation/QLocation>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QPlaceManagerEnginePrivate
{
public:
    QPlaceManagerEnginePrivate() = default;

    QString managerName;
    int managerVersion = -1;
    QPlaceManager *manager = nullptr;

private:
    Q_DISABLE_COPY(QPlaceManagerEnginePrivate)
};

QT_END_NAMESPACE

#endif // QPLACEMANAGERENGINE_P_H
