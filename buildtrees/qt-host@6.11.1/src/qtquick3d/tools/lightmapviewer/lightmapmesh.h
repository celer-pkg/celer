// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LIGHTMAPMESH_H
#define LIGHTMAPMESH_H

#include <QQuick3DGeometry>

#include <private/qquick3dmodel_p.h>

class LightmapMesh : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged FINAL)
    Q_PROPERTY(QQuick3DBounds3 bounds READ bounds NOTIFY boundsChanged)
    QML_ELEMENT

public:
    LightmapMesh(QQuick3DObject *parent = nullptr);
    ~LightmapMesh() override;

    QUrl source() const;
    void setSource(const QUrl &newSource);
    QString key() const;
    void setKey(const QString &newKey);
    const QQuick3DBounds3 &bounds() const;

signals:
    void sourceChanged();
    void keyChanged();
    void boundsChanged();

private:
    void updateData();

    QUrl m_source;
    QString m_key;

    QUrl m_currentlyLoadedSource;
    QString m_currentlyLoadedKey;

    QQuick3DBounds3 m_bounds;
};

#endif // LIGHTMAPMESH_H
