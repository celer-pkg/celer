// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPPINGMANAGERENGINE_H
#define QGEOMAPPINGMANAGERENGINE_H

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
#include <QList>
#include <QtLocation/private/qlocationglobal_p.h>

QT_BEGIN_NAMESPACE

class QLocale;
class QString;

class QGeoRectangle;
class QGeoCoordinate;
class QGeoCameraCapabilities;
class QGeoMappingManagerPrivate;
class QGeoMapRequestOptions;

class QGeoMappingManagerEnginePrivate;
class QGeoMap;
class QGeoMapType;

class Q_LOCATION_EXPORT QGeoMappingManagerEngine : public QObject
{
    Q_OBJECT

public:
    explicit QGeoMappingManagerEngine(QObject *parent = nullptr);
    virtual ~QGeoMappingManagerEngine();

    virtual QGeoMap *createMap() = 0;

    QVariantMap parameters() const;

    QString managerName() const;
    int managerVersion() const;

    QList<QGeoMapType> supportedMapTypes() const;

    // the class is private, so this can be virtual here for now.
    QGeoCameraCapabilities cameraCapabilities(int mapId = 0) const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    bool isInitialized() const;

Q_SIGNALS:
    void initialized();
    void supportedMapTypesChanged();

protected:
    void setSupportedMapTypes(const QList<QGeoMapType> &supportedMapTypes);
    void setCameraCapabilities(const QGeoCameraCapabilities &capabilities);

    void engineInitialized();

private:
    QGeoMappingManagerEnginePrivate *d_ptr;

    void setManagerName(const QString &managerName);
    void setManagerVersion(int managerVersion);

    Q_DECLARE_PRIVATE(QGeoMappingManagerEngine)
    Q_DISABLE_COPY(QGeoMappingManagerEngine)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
