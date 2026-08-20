// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef CUBOIDGEOMETRY_P_H
#define CUBOIDGEOMETRY_P_H

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

class Q_QUICK3DHELPERS_EXPORT CuboidGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(float xExtent READ xExtent WRITE setXExtent NOTIFY xExtentChanged FINAL)
    Q_PROPERTY(float yExtent READ yExtent WRITE setYExtent NOTIFY yExtentChanged FINAL)
    Q_PROPERTY(float zExtent READ zExtent WRITE setZExtent NOTIFY zExtentChanged FINAL)
    Q_PROPERTY(QSize yzMeshResolution READ yzMeshResolution WRITE setYzMeshResolution NOTIFY yzMeshResolutionChanged FINAL)
    Q_PROPERTY(QSize xzMeshResolution READ xzMeshResolution WRITE setXzMeshResolution NOTIFY xzMeshResolutionChanged FINAL)
    Q_PROPERTY(QSize xyMeshResolution READ xyMeshResolution WRITE setXyMeshResolution NOTIFY xyMeshResolutionChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 9)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    explicit CuboidGeometry(QQuick3DObject *parent = nullptr);
    ~CuboidGeometry() override;
    float xExtent() const;
    void setXExtent(float newXExtent);
    float yExtent() const;
    void setYExtent(float newYExtent);

    float zExtent() const;
    void setZExtent(float newZExtent);

    QSize yzMeshResolution() const;
    void setYzMeshResolution(const QSize &newYzMeshResolution);

    QSize xzMeshResolution() const;
    void setXzMeshResolution(const QSize &newXzMeshResolution);

    QSize xyMeshResolution() const;
    void setXyMeshResolution(const QSize &newXyMeshResolution);

    bool asynchronous() const;
    void setAsynchronous(bool newAsynchronous);

    Status status() const;

private Q_SLOTS:
    void doUpdateGeometry();
    void requestFinished();

Q_SIGNALS:
    void xExtentChanged();
    void yExtentChanged();
    void zExtentChanged();
    void yzMeshResolutionChanged();
    void xzMeshResolutionChanged();
    void xyMeshResolutionChanged();
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

    static CuboidGeometry::GeometryData generateCuboidGeometry(float xExtent,
                                                               float yExtent,
                                                               float zExtent,
                                                               QSize yzMeshResolution,
                                                               QSize xzMeshResolution,
                                                               QSize xyMeshResolution);
#if QT_CONFIG(concurrent)
    static void generateCuboidGeometryAsync(QPromise<CuboidGeometry::GeometryData> &promise,
                                            float xExtent,
                                            float yExtent,
                                            float zExtent,
                                            QSize yzMeshResolution,
                                            QSize xzMeshResolution,
                                            QSize xyMeshResolution);
#endif

    float m_xExtent = 100.0f;
    float m_yExtent = 100.0f;
    float m_zExtent = 100.0f;
    QSize m_yzMeshResolution = QSize(2, 2);
    QSize m_xzMeshResolution = QSize(2, 2);
    QSize m_xyMeshResolution = QSize(2, 2);
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

#endif // CUBOIDGEOMETRY_P_H
