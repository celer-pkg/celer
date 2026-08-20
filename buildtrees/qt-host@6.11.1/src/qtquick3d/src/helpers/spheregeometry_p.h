// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef SPHEREGEOMETRY_P_H
#define SPHEREGEOMETRY_P_H

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

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QQuick3DGeometry>
#include <QQmlEngine>
#include <QVector3D>

#if QT_CONFIG(concurrent)
#include <QFuture>
#include <QFutureWatcher>
#endif

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT SphereGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(float radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)
    Q_PROPERTY(int rings READ rings WRITE setRings NOTIFY ringsChanged FINAL)
    Q_PROPERTY(int segments READ segments WRITE setSegments NOTIFY segmentsChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 9)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    explicit SphereGeometry(QQuick3DObject *parent = nullptr);
    ~SphereGeometry() override;
    float radius() const;
    void setRadius(float newRadius);
    int rings() const;
    void setRings(int newRings);

    int segments() const;
    void setSegments(int newSegments);

    bool asynchronous() const;
    void setAsynchronous(bool newAsynchronous);

    Status status() const;

private Q_SLOTS:
    void doUpdateGeometry();
    void requestFinished();

Q_SIGNALS:
    void radiusChanged();
    void ringsChanged();
    void segmentsChanged();
    void asynchronousChanged();
    void statusChanged();

private:
    struct GeometryData {
        QByteArray vertexData;
        QByteArray indexData;
        QVector3D boundsMin;
        QVector3D boundsMax;
    };

    void scheduleGeometryUpdate();
    void updateGeometry(const GeometryData &geometryData);

    static SphereGeometry::GeometryData generateSphereGeometry(float radius,
                                                               int rings,
                                                               int segments);
#if QT_CONFIG(concurrent)
    static void generateSphereGeometryAsync(QPromise<SphereGeometry::GeometryData> &promise,
                                            float radius,
                                            int rings,
                                            int segments);
#endif

    float m_radius = 100.0f;
    int m_rings = 16;
    int m_segments = 32;
    bool m_asynchronous = true;
    Status m_status = Null;
#if QT_CONFIG(concurrent)
    QFuture<GeometryData> m_geometryDataFuture;
    QFutureWatcher<GeometryData> m_geometryDataWatcher;
#endif
    bool m_geometryUpdateRequested = false;
    bool m_pendingAsyncUpdate = false;
};

QT_END_NAMESPACE

#endif // SPHEREGEOMETRY_P_H
