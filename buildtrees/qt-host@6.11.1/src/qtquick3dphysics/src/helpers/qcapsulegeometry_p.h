// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CAPSULEGEOMETRY_H
#define CAPSULEGEOMETRY_H

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

#include <QQuick3DGeometry>

QT_BEGIN_NAMESPACE

class CapsuleGeometryPhysics : public QQuick3DGeometry
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CapsuleGeometry)
    Q_PROPERTY(bool enableNormals READ enableNormals WRITE setEnableNormals NOTIFY
                       enableNormalsChanged)
    Q_PROPERTY(bool enableUV READ enableUV WRITE setEnableUV NOTIFY enableUVChanged)

    Q_PROPERTY(int longitudes READ longitudes WRITE setLongitudes NOTIFY longitudesChanged)
    Q_PROPERTY(int latitudes READ latitudes WRITE setLatitudes NOTIFY latitudesChanged)
    Q_PROPERTY(int rings READ rings WRITE setRings NOTIFY ringsChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(float diameter READ diameter WRITE setDiameter NOTIFY diameterChanged)

public:
    CapsuleGeometryPhysics();

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

signals:
    void enableNormalsChanged();
    void enableUVChanged();
    void longitudesChanged();
    void latitudesChanged();
    void ringsChanged();
    void heightChanged();
    void diameterChanged();

private:
    enum class UvProfile { Fixed, Aspect, Uniform };

    void updateData();

    bool m_enableNormals = true;
    bool m_enableUV = false;

    // Number of longitudes, or meridians, distributed by azimuth
    int m_longitudes = 32;
    // Number of latitudes, distributed by inclination. Must be even
    int m_latitudes = 16;
    // Number of sections in cylinder between hemispheres
    int m_rings = 1;
    // Height of the middle cylinder on the y axis, excluding the hemispheres
    float m_height = 100.f;
    // Diameter on the xz plane
    float m_diameter = 100.f;
    UvProfile m_uvProfile = UvProfile::Fixed;
};

QT_END_NAMESPACE

#endif
