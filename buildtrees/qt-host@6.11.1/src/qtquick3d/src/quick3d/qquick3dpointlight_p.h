// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGPOINTLIGHT_H
#define QSSGPOINTLIGHT_H

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

#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DPointLight : public QQuick3DAbstractLight
{
    Q_OBJECT
    Q_PROPERTY(float constantFade READ constantFade WRITE setConstantFade NOTIFY constantFadeChanged)
    Q_PROPERTY(float linearFade READ linearFade WRITE setLinearFade NOTIFY linearFadeChanged)
    Q_PROPERTY(float quadraticFade READ quadraticFade WRITE setQuadraticFade NOTIFY quadraticFadeChanged)

    QML_NAMED_ELEMENT(PointLight)

public:
    explicit QQuick3DPointLight(QQuick3DNode *parent = nullptr);
    ~QQuick3DPointLight() override {}

    float constantFade() const;
    float linearFade() const;
    float quadraticFade() const;

public Q_SLOTS:
    void setConstantFade(float constantFade);
    void setLinearFade(float linearFade);
    void setQuadraticFade(float quadraticFade);

Q_SIGNALS:
    void constantFadeChanged();
    void linearFadeChanged();
    void quadraticFadeChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    float m_constantFade = 1.0f;
    float m_linearFade = 0.0f;
    float m_quadraticFade = 1.0f;
};

QT_END_NAMESPACE
#endif // QSSGPOINTLIGHT_H
