// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEFETCHER_P_H
#define QGEOTILEFETCHER_P_H

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

#include <QtCore/private/qobject_p.h>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QSize>
#include <QList>
#include <QMap>
#include <QLocale>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QHash>
#include "qgeomaptype_p.h"

QT_BEGIN_NAMESPACE

class QGeoTileSpec;
class QGeoTiledMapReply;
class QGeoMappingManagerEngine;

class Q_LOCATION_EXPORT QGeoTileFetcherPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGeoTileFetcher)
public:
    QBasicTimer timer_;
    QMutex queueMutex_;
    QList<QGeoTileSpec> queue_;
    QHash<QGeoTileSpec, QGeoTiledMapReply *> invmap_;
    QGeoMappingManagerEngine *engine_ = nullptr;
    bool enabled_ = false;
};

QT_END_NAMESPACE

#endif
