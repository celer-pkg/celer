// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LIGHTMAPFILE_H
#define LIGHTMAPFILE_H

#include <QObject>
#include <QList>
#include <QUrl>
#include <QDateTime>
#include <QSize>

#include <QtQuick3D/private/qquick3dtexturedata_p.h>

class LightmapFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList dataList READ dataList NOTIFY dataListChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString qtVersion READ qtVersion NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QString bakeStart READ bakeStart NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QString bakeDuration READ bakeDuration NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QVariantList options READ options NOTIFY metadataChanged FINAL)

public:
    explicit LightmapFile(QObject *parent = nullptr);

    QVariantList dataList() const;

    Q_INVOKABLE void loadData();
    Q_INVOKABLE QVariantList metadataFor(const QVariant &selectedEntry);
    Q_INVOKABLE QStringList keysReferencingMesh(const QString &meshKey) const;
    Q_INVOKABLE QQuick3DTextureData *textureDataFor(const QString &key, quint32 tag);
    Q_INVOKABLE QVariantList texturesAvailableFor(const QString &key) const;

    QUrl source() const;
    void setSource(const QUrl &newSource);

    QString qtVersion() const { return m_qtVersion; }
    QString bakeStart() const { return m_bakeStart; }
    QString bakeDuration() const { return m_bakeDuration; }
    QVariantList options() const { return m_options; }

signals:
    void dataListChanged();
    void sourceChanged();
    void metadataChanged();

private:
    QVariantList m_dataList;
    QUrl m_source;
    QString m_qtVersion;
    QString m_bakeStart;
    QString m_bakeDuration;
    QVariantList m_options;
};

#endif // LIGHTMAPFILE_H
