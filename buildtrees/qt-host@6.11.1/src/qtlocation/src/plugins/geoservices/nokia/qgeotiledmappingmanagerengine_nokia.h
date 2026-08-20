// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOTILEDMAPPINGMANAGERENGINE_NOKIA_H
#define QGEOTILEDMAPPINGMANAGERENGINE_NOKIA_H

#include "qgeotiledmappingmanagerengine_p.h"
#include <QtPositioning/QGeoRectangle>
#include "qgeomaptype_p.h"
#include "qgeomapversion.h"

#include <QGeoServiceProvider>

#include <QList>
#include <QHash>
#include <QSet>

QT_BEGIN_NAMESPACE

class QByteArray;
class QGeoTileSpec;
class QGeoNetworkAccessManager;

class QGeoTiledMappingManagerEngineNokia : public QGeoTiledMappingManagerEngine
{
    Q_OBJECT

public:
    QGeoTiledMappingManagerEngineNokia(QGeoNetworkAccessManager *networkManager,
                                       const QVariantMap &parameters,
                                       QGeoServiceProvider::Error *error,
                                       QString *errorString);
    ~QGeoTiledMappingManagerEngineNokia();

    QGeoMap *createMap() override;
    QString evaluateCopyrightsText(const QGeoMapType &mapType,
                                   qreal zoomLevel,
                                   const QSet<QGeoTileSpec> &tiles);
    QString getScheme(int mapId) const;
    QString getBaseScheme(int mapId) const;
    int mapVersion() const;

public Q_SLOTS:
    void loadCopyrightsDescriptorsFromJson(const QByteArray &jsonData);
    void parseNewVersionInfo(const QByteArray &versionData);

private:
    class CopyrightDesc
    {
    public:
        CopyrightDesc()
            : maxLevel(-1),
              minLevel(-1) {}

        qreal maxLevel;
        qreal minLevel;
        QList<QGeoRectangle> boxes;
        QString alt;
        QString label;
    };

    void initialize();
    void populateMapSchemes();
    void updateVersion(const QJsonObject &newVersionData);
    void saveMapVersion();
    void loadMapVersion();

    QHash<QString, QList<CopyrightDesc> > m_copyrights;
    QHash<int, QString> m_mapSchemes;
    QGeoMapVersion m_mapVersion;

    QString m_cacheDirectory;
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPPINGMANAGERENGINE_NOKIA_H
