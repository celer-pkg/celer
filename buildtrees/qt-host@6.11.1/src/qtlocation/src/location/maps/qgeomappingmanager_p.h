// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPPINGMANAGER_H
#define QGEOMAPPINGMANAGER_H

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

#include <QObject>
#include <QtLocation/private/qlocationglobal_p.h>

QT_BEGIN_NAMESPACE

class QGeoMap;
class QLocale;
class QGeoRectangle;
class QGeoCoordinate;
class QGeoMapType;
class QGeoMappingManagerPrivate;
class QGeoMapRequestOptions;
class QGeoMappingManagerEngine;
class QGeoCameraCapabilities;


class Q_LOCATION_EXPORT QGeoMappingManager : public QObject
{
    Q_OBJECT

public:
    ~QGeoMappingManager();

    QString managerName() const;
    int managerVersion() const;

    QGeoMap *createMap(QObject *parent);

    QList<QGeoMapType> supportedMapTypes() const;

    bool isInitialized() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

Q_SIGNALS:
    void initialized();
    void supportedMapTypesChanged();

protected:
    QGeoMappingManager(QGeoMappingManagerEngine *engine, QObject *parent = nullptr);

private:
    QGeoMappingManagerPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoMappingManager)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
