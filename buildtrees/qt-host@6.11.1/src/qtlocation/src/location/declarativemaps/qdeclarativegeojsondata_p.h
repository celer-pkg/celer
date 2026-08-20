// Copyright (C) 2019 Julian Sherollari <jdotsh@gmail.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOJSONFILE_H
#define QDECLARATIVEGEOJSONFILE_H

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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtLocation/private/qdeclarativegeomapitemview_p.h>
#include <QtLocation/private/qdeclarativegeomapquickitem_p.h>
#include <QtLocation/private/qdeclarativecirclemapitem_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p.h>
#include <QtLocation/private/qdeclarativerectanglemapitem_p.h>

#include <QtQml/qqml.h>
#include <QtQml/QQmlParserStatus>

QT_BEGIN_NAMESPACE


class Q_LOCATION_EXPORT QDeclarativeGeoJsonData : public QObject
{
    Q_OBJECT
    QML_ADDED_IN_VERSION(6, 7)
    QML_NAMED_ELEMENT(GeoJsonData)
    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QUrl sourceUrl READ sourceUrl WRITE openUrl NOTIFY sourceUrlChanged)

public:
    explicit QDeclarativeGeoJsonData(QObject *parent = nullptr);
    virtual ~QDeclarativeGeoJsonData();

    QVariant model() const;
    void setModel(const QVariant &model);

    QUrl sourceUrl() const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void addItem(QQuickItem *item);
    Q_INVOKABLE bool open();
    Q_INVOKABLE bool openUrl(const QUrl &url);
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool saveAs(const QUrl &url);
    Q_INVOKABLE void setModelToMapContents(QDeclarativeGeoMap *map);

signals:
    void modelChanged();
    void sourceUrlChanged();

private:
    static QVariantList toVariant(QDeclarativeGeoMap *mapItemView);
    static bool dumpGeoJSON(const QVariantList &geoJson, const QUrl &url);
    static bool writeDebug(const QVariantList &geoJson, const QUrl &url);
    static QString toString(const QVariantList &geoJson);

    QVariant m_content;
    QUrl m_url;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGeoJsonData)

#endif /* QDECLARATIVEGEOJSONFILE_H */
