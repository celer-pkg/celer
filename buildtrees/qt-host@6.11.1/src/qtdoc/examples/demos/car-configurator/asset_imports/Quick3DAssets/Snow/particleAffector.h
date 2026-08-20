// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PARTICLEAFFECTOR_H
#define PARTICLEAFFECTOR_H

#include <QtQuick3DParticles/private/qquick3dparticleaffector_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleemitter_p.h>
#include <QVector3D>


class ShapeAffector3D : public QQuick3DParticleAffector
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ShapeType shapeType READ shapeType WRITE setShapeType NOTIFY shapeTypeChanged)
    Q_PROPERTY(bool inverted READ inverted WRITE setInverted NOTIFY invertedChanged FINAL)
    Q_PROPERTY(QVector3D targetPosition READ targetPosition WRITE setTargetPosition NOTIFY targetPositionChanged)
    Q_PROPERTY(QVector3D targetVelocity READ targetVelocity WRITE setTargetVelocity NOTIFY targetVelocityChanged)
    Q_PROPERTY(QVector3D targetRotation READ targetRotation WRITE setTargetRotation NOTIFY targetRotationChanged)
    Q_PROPERTY(QVector3D targetScale READ targetScale WRITE setTargetScale NOTIFY targetScaleChanged)
    Q_PROPERTY(QColor targetColor READ targetColor WRITE setTargetColor NOTIFY targetColorChanged)
    Q_PROPERTY(AffectFlags affectFlags READ affectFlags WRITE setAffectFlags NOTIFY affectFlagsChanged)

public:
    enum ShapeType {
        Sphere,
        Box,
        Cylinder
    };
    Q_ENUM(ShapeType)

    enum AffectFlag {
        AffectNone     = 0x00,
        AffectPosition = 0x01,
        AffectVelocity = 0x02,
        AffectRotation = 0x04,
        AffectScale    = 0x08,
        AffectColor    = 0x10
    };
    Q_DECLARE_FLAGS(AffectFlags, AffectFlag)
    Q_FLAG(AffectFlags)

    explicit ShapeAffector3D(QQuick3DNode *parent = nullptr);

    ShapeType shapeType() const { return m_shapeType; }
    void setShapeType(ShapeType type);

    bool inverted() const { return m_inverted; }
    void setInverted(bool newInverted);

    QVector3D targetPosition() const { return m_targetPosition; }
    void setTargetPosition(const QVector3D &position);

    QVector3D targetVelocity() const { return m_targetVelocity; }
    void setTargetVelocity(const QVector3D &velocity);

    QVector3D targetRotation() const { return m_targetRotation; }
    void setTargetRotation(const QVector3D &rotation);

    QVector3D targetScale() const { return m_targetScale; }
    void setTargetScale(const QVector3D &scale);

    QColor targetColor() const { return m_targetColor; }
    void setTargetColor(const QColor &color);

    AffectFlags affectFlags() const { return m_affectFlags; }
    void setAffectFlags(AffectFlags flags);

protected:
    void affectParticle(const QQuick3DParticleData &sd,
                        QQuick3DParticleDataCurrent *d,
                        float time) override;

signals:
    void shapeTypeChanged();
    void invertedChanged();
    void targetPositionChanged();
    void targetVelocityChanged();
    void targetRotationChanged();
    void targetScaleChanged();
    void targetColorChanged();
    void affectFlagsChanged();

private slots:
    void updateInverseTransform();

private:
    bool isInsideShape(const QVector3D &worldPosition);
    bool isInsideSphere(const QVector3D &localPosition);
    bool isInsideBox(const QVector3D &localPosition);
    bool isInsideCylinder(const QVector3D &localPosition);
    QVector3D worldToLocal(const QVector3D &worldPosition);

    ShapeType m_shapeType = Sphere;
    QMatrix4x4 m_inverseTransform;
    bool m_inverted = false;

    QVector3D m_targetPosition{0, 0, 0};
    QVector3D m_targetVelocity{0, 0, 0};
    QVector3D m_targetRotation{0, 0, 0};
    QVector3D m_targetScale{1, 1, 1};
    QColor m_targetColor{0, 0, 0, 0};

    AffectFlags m_affectFlags = AffectColor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShapeAffector3D::AffectFlags)

#endif // PARTICLEAFFECTOR_H
