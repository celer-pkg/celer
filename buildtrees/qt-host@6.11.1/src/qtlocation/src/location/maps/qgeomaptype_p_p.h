// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPTYPE_P_H
#define QGEOMAPTYPE_P_H

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

#include <QMetaType>
#include <QString>
#include <QVariantMap>
#include <QByteArray>
#include <QSharedData>
#include "qgeocameracapabilities_p.h"
#include "qgeomaptype_p.h"

QT_BEGIN_NAMESPACE

class QGeoMapTypePrivate : public QSharedData
{
public:
    QGeoMapTypePrivate();
    QGeoMapTypePrivate(QGeoMapType::MapStyle style, const QString &name, const QString &description, bool mobile,
                       bool night, int mapId, const QByteArray &pluginName,
                       const QGeoCameraCapabilities &cameraCapabilities,
                       const QVariantMap &metadata);

    bool operator==(const QGeoMapTypePrivate &other) const;

    QString name_;
    QString description_;
    QByteArray pluginName_;
    QGeoCameraCapabilities cameraCapabilities_;
    QVariantMap metadata_;
    QGeoMapType::MapStyle style_ = QGeoMapType::NoMap;
    int mapId_ = 0;
    bool mobile_ = false;
    bool night_ = false;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGeoMapTypePrivate)

#endif // QGEOMAPTYPE_P_H
