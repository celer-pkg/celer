// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DSPOTLIGHT_H
#define QQUICK3DSPOTLIGHT_H

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

#include <QtQuick3D/private/qquick3dabstractlight_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DSpotLight : public QQuick3DAbstractLight
{
    Q_OBJECT
    Q_PROPERTY(float constantFade READ constantFade WRITE setConstantFade NOTIFY constantFadeChanged)
    Q_PROPERTY(float linearFade READ linearFade WRITE setLinearFade NOTIFY linearFadeChanged)
    Q_PROPERTY(float quadraticFade READ quadraticFade WRITE setQuadraticFade NOTIFY quadraticFadeChanged)
    Q_PROPERTY(float coneAngle READ coneAngle WRITE setConeAngle NOTIFY coneAngleChanged)
    Q_PROPERTY(float innerConeAngle READ innerConeAngle WRITE setInnerConeAngle NOTIFY innerConeAngleChanged)

    QML_NAMED_ELEMENT(SpotLight)

public:
    explicit QQuick3DSpotLight(QQuick3DNode *parent = nullptr);
    ~QQuick3DSpotLight() override {}

    float constantFade() const;
    float linearFade() const;
    float quadraticFade() const;
    float coneAngle() const;
    float innerConeAngle() const;

public Q_SLOTS:
    void setConstantFade(float constantFade);
    void setLinearFade(float linearFade);
    void setQuadraticFade(float quadraticFade);
    void setConeAngle(float coneAngle);
    void setInnerConeAngle(float innerConeAngle);

Q_SIGNALS:
    void constantFadeChanged();
    void linearFadeChanged();
    void quadraticFadeChanged();
    void coneAngleChanged();
    void innerConeAngleChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    float m_constantFade = 1.0f;
    float m_linearFade = 0.0f;
    float m_quadraticFade = 1.0f;
    float m_coneAngle = 40.0f;
    float m_innerConeAngle = 30.0f;
};

QT_END_NAMESPACE

#endif // QQUICK3DSPOTLIGHT_H
