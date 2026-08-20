// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEPARSEROSRMV5_H
#define QGEOROUTEPARSEROSRMV5_H

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


#include <QtLocation/private/qgeorouteparser_p.h>
#include <QtCore/QUrlQuery>

QT_BEGIN_NAMESPACE

class QGeoRouteParserOsrmV5Private;

class Q_LOCATION_EXPORT QGeoRouteParserOsrmV5Extension
{
public:
    QGeoRouteParserOsrmV5Extension()
    {
    }

    virtual ~QGeoRouteParserOsrmV5Extension()
    {
    }

    virtual void updateQuery(QUrlQuery &query) const = 0;
    virtual void updateSegment(QGeoRouteSegment &segment, const QJsonObject &step, const QJsonObject &maneuver) const = 0;
};

class Q_LOCATION_EXPORT QGeoRouteParserOsrmV5 : public QGeoRouteParser
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoRouteParserOsrmV5)

public:
    QGeoRouteParserOsrmV5(QObject *parent = nullptr);
    virtual ~QGeoRouteParserOsrmV5();

    void setExtension(const QGeoRouteParserOsrmV5Extension *extension);

private:
    Q_DISABLE_COPY(QGeoRouteParserOsrmV5)
};

QT_END_NAMESPACE

#endif // QGEOROUTEPARSEROSRMV5_H
