// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef PLANEGEOMETRY_P_H
#define PLANEGEOMETRY_P_H

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

class Q_QUICK3DHELPERS_EXPORT PlaneGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(float width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged FINAL)
    Q_PROPERTY(QSize meshResolution READ meshResolution WRITE setMeshResolution NOTIFY meshResolutionChanged FINAL)
    Q_PROPERTY(Plane plane READ plane WRITE setPlane NOTIFY planeChanged FINAL)
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY reversedChanged FINAL)
    Q_PROPERTY(bool mirrored READ mirrored WRITE setMirrored NOTIFY mirroredChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 9)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    enum Plane { XY, XZ, ZY };
    Q_ENUM(Plane)

    explicit PlaneGeometry(QQuick3DObject *parent = nullptr);
    ~PlaneGeometry() override;

    float width() const;
    void setWidth(float newWidth);
    float height() const;
    void setHeight(float newHeight);

    QSize meshResolution() const;
    void setMeshResolution(const QSize &newMeshResolution);

    Plane plane() const;
    void setPlane(Plane newPlane);

    bool mirrored() const;
    void setMirrored(bool newMirrored);

    bool asynchronous() const;
    void setAsynchronous(bool newAsynchronous);

    Status status() const;

    bool reversed() const;
    void setReversed(bool newReversed);

private Q_SLOTS:
    void doUpdateGeometry();
    void requestFinished();

Q_SIGNALS:
    void widthChanged();
    void heightChanged();
    void meshResolutionChanged();
    void planeChanged();
    void mirroredChanged();
    void asynchronousChanged();
    void statusChanged();
    void reversedChanged();

private:
    struct GeometryData {
        QByteArray vertexData;
        QByteArray indexData;
        QVector3D boundsMin;
        QVector3D boundsMax;
    };

    void scheduleGeometryUpdate();
    void updateGeometry(const GeometryData &geometryData);

    static PlaneGeometry::GeometryData generatePlaneGeometry(float width,
                                                             float height,
                                                             QSize meshResolution,
                                                             Plane plane,
                                                             bool reversed,
                                                             bool mirrored);
#if QT_CONFIG(concurrent)
    static void generatePlaneGeometryAsync(QPromise<PlaneGeometry::GeometryData> &promise,
                                           float width,
                                           float height,
                                           QSize meshResolution,
                                           Plane plane,
                                           bool reversed,
                                           bool mirrored);
#endif
    float m_width = 100.0f;
    float m_height = 100.0f;
    QSize m_meshResolution = QSize(2, 2);
    Plane m_plane = XY;
    bool m_reversed = false;
    bool m_mirrored = false;
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

#endif // PLANEGEOMETRY_P_H
