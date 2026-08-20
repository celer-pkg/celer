// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODINGMANAGER_H
#define QGEOCODINGMANAGER_H

#include <QtLocation/QGeoCodeReply>
#include <QtPositioning/QGeoShape>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QLocale;

class QGeoAddress;
class QGeoCodingManagerEngine;
class QGeoCodingManagerPrivate;

class Q_LOCATION_EXPORT QGeoCodingManager : public QObject
{
    Q_OBJECT
public:
    ~QGeoCodingManager();

    QString managerName() const;
    int managerVersion() const;

    QGeoCodeReply *geocode(const QGeoAddress &address,
                           const QGeoShape &bounds = QGeoShape());
    QGeoCodeReply *geocode(const QString &searchString,
                            int limit = -1,
                            int offset = 0,
                           const QGeoShape &bounds = QGeoShape());

    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds = QGeoShape());

    void setLocale(const QLocale &locale);
    QLocale locale() const;

Q_SIGNALS:
    void finished(QGeoCodeReply *reply);
    void errorOccurred(QGeoCodeReply *reply, QGeoCodeReply::Error error,
                       const QString &errorString = QString());

private:
    explicit QGeoCodingManager(QGeoCodingManagerEngine *engine, QObject *parent = nullptr);

    QGeoCodingManagerPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoCodingManager)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
