// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEDMAPPINGMANAGERENGINEOSM_H
#define QGEOTILEDMAPPINGMANAGERENGINEOSM_H

#include "qgeotileproviderosm.h"

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

#include <QList>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineOsm : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

    friend class QGeoTiledMapOsm;
public:
    QGeoTiledMappingManagerEngineOsm(const QVariantMap &parameters,
                                     QGeoServiceProvider::Error *error, QString *errorString);
    ~QGeoTiledMappingManagerEngineOsm();

    QGeoMap *createMap() override;
    const QList<QGeoTileProviderOsm *> &providers();
    QString customCopyright() const;

protected Q_SLOTS:
    void onProviderResolutionFinished(const QGeoTileProviderOsm *provider);
    void onProviderResolutionError(const QGeoTileProviderOsm *provider);

protected:
    void updateMapTypes();

private:
    QList<QGeoTileProviderOsm *> m_providers;
    QString m_customCopyright;
    QString m_cacheDirectory;
    QString m_offlineDirectory;
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPPINGMANAGERENGINEOSM_H
