// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPPINGMANAGERENGINE_P_H
#define QGEOMAPPINGMANAGERENGINE_P_H

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

#include <QList>
#include <QLocale>
#include "qgeomappingmanager_p.h"
#include "qgeocameracapabilities_p.h"

QT_BEGIN_NAMESPACE

class QGeoMapType;
class QGeoTileSpec;
class QGeoTiledMapReply;

class QGeoMappingManagerEnginePrivate
{
public:
    QGeoMappingManagerEnginePrivate();
    ~QGeoMappingManagerEnginePrivate();

    QString managerName;
    QList<QGeoMapType> supportedMapTypes;
    QLocale locale;
    QGeoCameraCapabilities capabilities_;

    bool initialized = false;
    int managerVersion = -1;

private:
    Q_DISABLE_COPY(QGeoMappingManagerEnginePrivate)
};

QT_END_NAMESPACE

#endif
