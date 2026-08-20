// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DFOG_P_H
#define QQUICK3DFOG_P_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DFog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float density READ density WRITE setDensity NOTIFY densityChanged)
    Q_PROPERTY(bool depthEnabled READ isDepthEnabled WRITE setDepthEnabled NOTIFY depthEnabledChanged)
    Q_PROPERTY(float depthNear READ depthNear WRITE setDepthNear NOTIFY depthNearChanged)
    Q_PROPERTY(float depthFar READ depthFar WRITE setDepthFar NOTIFY depthFarChanged)
    Q_PROPERTY(float depthCurve READ depthCurve WRITE setDepthCurve NOTIFY depthCurveChanged)
    Q_PROPERTY(bool heightEnabled READ isHeightEnabled WRITE setHeightEnabled NOTIFY heightEnabledChanged)
    Q_PROPERTY(float leastIntenseY READ leastIntenseY WRITE setLeastIntenseY NOTIFY leastIntenseYChanged)
    Q_PROPERTY(float mostIntenseY READ mostIntenseY WRITE setMostIntenseY NOTIFY mostIntenseYChanged)
    Q_PROPERTY(float heightCurve READ heightCurve WRITE setHeightCurve NOTIFY heightCurveChanged)
    Q_PROPERTY(bool transmitEnabled READ isTransmitEnabled WRITE setTransmitEnabled NOTIFY transmitEnabledChanged)
    Q_PROPERTY(float transmitCurve READ transmitCurve WRITE setTransmitCurve NOTIFY transmitCurveChanged)

    QML_NAMED_ELEMENT(Fog)

public:
    bool isEnabled() const;
    QColor color() const;
    float density() const;
    bool isDepthEnabled() const;
    float depthNear() const;
    float depthFar() const;
    float depthCurve() const;
    bool isHeightEnabled() const;
    float leastIntenseY() const;
    float mostIntenseY() const;
    float heightCurve() const;
    bool isTransmitEnabled() const;
    float transmitCurve() const;

public Q_SLOTS:
    void setEnabled(bool newEnabled);
    void setColor(const QColor &newColor);
    void setDensity(float newDensity);
    void setDepthEnabled(bool newDepthEnabled);
    void setDepthNear(float newDepthNear);
    void setDepthFar(float newDepthFar);
    void setDepthCurve(float newDepthCurve);
    void setHeightEnabled(bool newHeightEnabled);
    void setLeastIntenseY(float newleastIntenseY);
    void setMostIntenseY(float newmostIntenseY);
    void setHeightCurve(float newHeightCurve);
    void setTransmitEnabled(bool newTransmitEnabled);
    void setTransmitCurve(float newTransmitCurve);

Q_SIGNALS:
    void changed();
    void enabledChanged();
    void colorChanged();
    void densityChanged();
    void depthEnabledChanged();
    void depthNearChanged();
    void depthFarChanged();
    void depthCurveChanged();
    void heightEnabledChanged();
    void leastIntenseYChanged();
    void mostIntenseYChanged();
    void heightCurveChanged();
    void transmitEnabledChanged();
    void transmitCurveChanged();

private:
    bool m_enabled = false;
    QColor m_color = QColor::fromRgbF(0.5f, 0.6f, 0.7f, 1.0f);
    float m_density = 1.0f;
    bool m_depthEnabled = false;
    float m_depthNear = 10.0f;
    float m_depthFar = 1000.0f;
    float m_depthCurve = 1.0f;
    bool m_heightEnabled = false;
    float m_leastIntenseY = 10.0f;
    float m_mostIntenseY = 0.0f;
    float m_heightCurve = 1.0f;
    bool m_transmitEnabled = false;
    float m_transmitCurve = 1.0f;
};

QT_END_NAMESPACE

#endif // QQUICK3DFOG_P_H
