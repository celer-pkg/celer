// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef PILLGEOMETRY_P_H
#define PILLGEOMETRY_P_H

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

class Q_QUICK3DHELPERS_EXPORT CapsuleGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(bool enableNormals READ enableNormals WRITE setEnableNormals NOTIFY enableNormalsChanged)
    Q_PROPERTY(bool enableUV READ enableUV WRITE setEnableUV NOTIFY enableUVChanged)
    Q_PROPERTY(int longitudes READ longitudes WRITE setLongitudes NOTIFY longitudesChanged)
    Q_PROPERTY(int latitudes READ latitudes WRITE setLatitudes NOTIFY latitudesChanged)
    Q_PROPERTY(int rings READ rings WRITE setRings NOTIFY ringsChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(float diameter READ diameter WRITE setDiameter NOTIFY diameterChanged)
    Q_PROPERTY(UVProfile uvProfile READ uvProfile WRITE setUVProfile NOTIFY uvProfileChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 10)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)
    enum UVProfile { Fixed, Aspect, Uniform };
    Q_ENUM(UVProfile)

    explicit CapsuleGeometry(QQuick3DObject *parent = nullptr);
    ~CapsuleGeometry() override;

    bool enableNormals() const { return m_enableNormals; }
    void setEnableNormals(bool enable);

    bool enableUV() const { return m_enableUV; }
    void setEnableUV(bool enable);

    int longitudes() const { return m_longitudes; }
    void setLongitudes(int longitudes);

    int latitudes() const { return m_latitudes; }
    void setLatitudes(int latitudes);

    int rings() const { return m_rings; }
    void setRings(int rings);

    float height() const { return m_height; }
    void setHeight(float height);

    float diameter() const { return m_diameter; }
    void setDiameter(float diameter);

    UVProfile uvProfile() const;
    void setUVProfile(UVProfile newUVProfile);

    bool asynchronous() const;
    void setAsynchronous(bool newAsynchronous);

    Status status() const;

private Q_SLOTS:
    void doUpdateGeometry();
    void requestFinished();

Q_SIGNALS:
    void enableNormalsChanged();
    void enableUVChanged();
    void longitudesChanged();
    void latitudesChanged();
    void ringsChanged();
    void heightChanged();
    void diameterChanged();
    void uvProfileChanged();
    void asynchronousChanged();
    void statusChanged();

private:
    struct GeometryData
    {
        QByteArray vertexData;
        QByteArray indexData;
        QVector3D boundsMin;
        QVector3D boundsMax;
        uint32_t stride = 0;
        uint32_t strideNormal = 0;
        uint32_t strideUV = 0;
        bool enableNormals = false;
        bool enableUV = false;
    };

    void scheduleGeometryUpdate();
    void updateGeometry(const GeometryData &geometryData);

    static CapsuleGeometry::GeometryData generateCapsuleGeometry(bool enableNormals,
                                                                 bool enableUV,
                                                                 int longitudes,
                                                                 int latitudes,
                                                                 int rings,
                                                                 float height,
                                                                 float diameter,
                                                                 UVProfile uvProfile);
#if QT_CONFIG(concurrent)
    static void generateCapsuleGeometryAsync(QPromise<CapsuleGeometry::GeometryData> &promise,
                                             bool enableNormals,
                                             bool enableUV,
                                             int longitudes,
                                             int latitudes,
                                             int rings,
                                             float height,
                                             float diameter,
                                             UVProfile uvProfile);
#endif

    void updateData();

    bool m_enableNormals = true;
    bool m_enableUV = false;

    int m_longitudes = 32;
    int m_latitudes = 16;
    int m_rings = 1;
    float m_height = 100.f;
    float m_diameter = 100.f;
    UVProfile m_uvProfile = UVProfile::Fixed;

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

#endif // PILLGEOMETRY_P_H
