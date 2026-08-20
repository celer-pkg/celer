// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEPARSEROSRMV4_H
#define QGEOROUTEPARSEROSRMV4_H

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

QT_BEGIN_NAMESPACE

class QGeoRouteParserOsrmV4Private;
class QGeoRouteParserOsrmV4 : public QGeoRouteParser
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoRouteParserOsrmV4)

public:
    QGeoRouteParserOsrmV4(QObject *parent = nullptr);
    virtual ~QGeoRouteParserOsrmV4();

private:
    Q_DISABLE_COPY(QGeoRouteParserOsrmV4)
};

QT_END_NAMESPACE

#endif // QGEOROUTEPARSEROSRMV4_H
