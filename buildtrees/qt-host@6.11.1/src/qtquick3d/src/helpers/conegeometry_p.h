// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef CONEGEOMETRY_P_H
#define CONEGEOMETRY_P_H

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

class Q_QUICK3DHELPERS_EXPORT ConeGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(float topRadius READ topRadius WRITE setTopRadius NOTIFY topRadiusChanged FINAL)
    Q_PROPERTY(float bottomRadius READ bottomRadius WRITE setBottomRadius NOTIFY bottomRadiusChanged FINAL)
    Q_PROPERTY(float length READ length WRITE setLength NOTIFY lengthChanged FINAL)
    Q_PROPERTY(int rings READ rings WRITE setRings NOTIFY ringsChanged FINAL)
    Q_PROPERTY(int segments READ segments WRITE setSegments NOTIFY segmentsChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 9)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    explicit ConeGeometry(QQuick3DObject *parent = nullptr);
    ~ConeGeometry() override;
    float topRadius() const;
    void setTopRadius(float newTopRadius);
    float bottomRadius() const;
    void setBottomRadius(float newBottomRadius);

    float length() const;
    void setLength(float newLength);

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
    void topRadiusChanged();
    void bottomRadiusChanged();
    void lengthChanged();
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

    static ConeGeometry::GeometryData generateConeGeometry(float topRadius,
                                                           float bottomRadius,
                                                           float length,
                                                           int rings,
                                                           int slices);
#if QT_CONFIG(concurrent)
    static void generateConeGeometryAsync(QPromise<ConeGeometry::GeometryData> &promise,
                                          float topRadius,
                                          float bottomRadius,
                                          float length,
                                          int rings,
                                          int segments);
#endif

    float m_topRadius = 0.0f;
    float m_bottomRadius = 50.0f;
    float m_length = 100.0f;
    int m_rings = 0;
    int m_segments = 20;
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

#endif // CONEGEOMETRY_P_H
