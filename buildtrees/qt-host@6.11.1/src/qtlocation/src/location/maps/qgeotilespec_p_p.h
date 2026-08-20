// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOTILESPEC_P_H
#define QGEOTILESPEC_P_H

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

#include <QtCore/qhashfunctions.h>
#include <QString>
#include <QSharedData>

QT_BEGIN_NAMESPACE

class QGeoTileSpecPrivate : public QSharedData
{
public:
    QGeoTileSpecPrivate(const QString &plugin = {}, int mapId = 0,
                                  int zoom = -1, int x = -1, int y = -1, int version = -1)
        : plugin_(plugin), mapId_(mapId), zoom_(zoom),
          x_(x), y_(y), version_(version)
    {}


    size_t hash(size_t seed) const
    {
        return qHashMulti(seed,
                          mapId_,
                          zoom_,
                          x_,
                          y_,
                          version_,
                          plugin_);
    }

    inline bool operator==(const QGeoTileSpecPrivate &rhs) const
    {
        return mapId_ == rhs.mapId_
            && zoom_ == rhs.zoom_
            && x_ == rhs.x_
            && y_ == rhs.y_
            && version_ == rhs.version_
            && plugin_ == rhs.plugin_;
    }
    bool operator<(const QGeoTileSpecPrivate &rhs) const;

    QString plugin_;
    int mapId_ = 0;
    int zoom_ = -1;
    int x_ = -1;
    int y_ = -1;
    int version_ = -1;
};

QT_END_NAMESPACE

#endif // QGEOTILESPEC_P_H
