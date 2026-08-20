// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODINGMANAGERENGINE_H
#define QGEOCODINGMANAGERENGINE_H

#include <QtLocation/qlocationglobal.h>
#include <QtCore/QObject>
#include <QtLocation/QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QGeoAddress;
class QGeoCoordinate;
class QGeoShape;
class QGeoCodingManagerEnginePrivate;

class Q_LOCATION_EXPORT QGeoCodingManagerEngine : public QObject
{
    Q_OBJECT
public:
    explicit QGeoCodingManagerEngine(const QVariantMap &parameters, QObject *parent = nullptr);
    virtual ~QGeoCodingManagerEngine();

    QString managerName() const;
    int managerVersion() const;

    virtual QGeoCodeReply *geocode(const QGeoAddress &address, const QGeoShape &bounds);
    virtual QGeoCodeReply *geocode(const QString &address,
                                   int limit,
                                   int offset,
                                   const QGeoShape &bounds);
    virtual QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                          const QGeoShape &bounds);


    void setLocale(const QLocale &locale);
    QLocale locale() const;

Q_SIGNALS:
    void finished(QGeoCodeReply *reply);
    void errorOccurred(QGeoCodeReply *reply, QGeoCodeReply::Error error,
                       const QString &errorString = QString());

private:
    void setManagerName(const QString &managerName);
    void setManagerVersion(int managerVersion);

    QGeoCodingManagerEnginePrivate *d_ptr;
    Q_DISABLE_COPY(QGeoCodingManagerEngine)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
