// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOTILEDMAPPINGMANAGERENGINEESRI_H
#define GEOTILEDMAPPINGMANAGERENGINEESRI_H

#include <QGeoServiceProvider>

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

#include "geomapsource.h"

QT_BEGIN_NAMESPACE

class GeoTiledMappingManagerEngineEsri : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    GeoTiledMappingManagerEngineEsri(const QVariantMap &parameters,
                                     QGeoServiceProvider::Error *error, QString *errorString);
    virtual ~GeoTiledMappingManagerEngineEsri();

    QGeoMap *createMap() override;

    inline const QList<GeoMapSource *>& mapSources() const;
    GeoMapSource *mapSource(int mapId) const;

private:
    bool initializeMapSources(QGeoServiceProvider::Error *error, QString *errorString, const QGeoCameraCapabilities &cameraCaps);

    QList<GeoMapSource *> m_mapSources;
};

inline const QList<GeoMapSource *>& GeoTiledMappingManagerEngineEsri::mapSources() const
{
    return m_mapSources;
}

QT_END_NAMESPACE

#endif // GEOTILEDMAPPINGMANAGERENGINEESRI_H
