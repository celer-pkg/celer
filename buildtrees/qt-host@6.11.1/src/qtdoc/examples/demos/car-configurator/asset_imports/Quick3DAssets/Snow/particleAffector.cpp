// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "particleAffector.h"
#include <QtGlobal>

ShapeAffector3D::ShapeAffector3D(QQuick3DNode *parent)
    : QQuick3DParticleAffector(parent)
{
    connect(this, &QQuick3DNode::positionChanged, this, &ShapeAffector3D::updateInverseTransform);
    connect(this, &QQuick3DNode::rotationChanged, this, &ShapeAffector3D::updateInverseTransform);
    connect(this, &QQuick3DNode::scaleChanged, this, &ShapeAffector3D::updateInverseTransform);
    updateInverseTransform();
}

void ShapeAffector3D::setShapeType(ShapeType type)
{
    if (m_shapeType != type) {
        m_shapeType = type;
        emit shapeTypeChanged();
    }
}

void ShapeAffector3D::setInverted(bool newInverted)
{
    if (m_inverted == newInverted)
        return;
    m_inverted = newInverted;
    emit invertedChanged();
}

void ShapeAffector3D::setTargetPosition(const QVector3D &position)
{
    if (m_targetPosition != position) {
        m_targetPosition = position;
        emit targetPositionChanged();
    }
}

void ShapeAffector3D::setTargetVelocity(const QVector3D &velocity)
{
    if (m_targetVelocity != velocity) {
        m_targetVelocity = velocity;
        emit targetVelocityChanged();
    }
}

void ShapeAffector3D::setTargetRotation(const QVector3D &rotation)
{
    if (m_targetRotation != rotation) {
        m_targetRotation = rotation;
        emit targetRotationChanged();
    }
}

void ShapeAffector3D::setTargetScale(const QVector3D &scale)
{
    if (m_targetScale != scale) {
        m_targetScale = scale;
        emit targetScaleChanged();
    }
}

void ShapeAffector3D::setTargetColor(const QColor &color)
{
    if (m_targetColor != color) {
        m_targetColor = color;
        emit targetColorChanged();
    }
}

void ShapeAffector3D::setAffectFlags(AffectFlags flags)
{
    if (m_affectFlags != flags) {
        m_affectFlags = flags;
        emit affectFlagsChanged();
    }
}

void ShapeAffector3D::updateInverseTransform()
{
    QMatrix4x4 transform;
    transform.translate(position());
    transform.rotate(rotation());
    transform.scale(scale());
    m_inverseTransform = transform.inverted();
}

void ShapeAffector3D::affectParticle(const QQuick3DParticleData &sd,
                                     QQuick3DParticleDataCurrent *d,
                                     float time)
{
    Q_UNUSED(sd);
    Q_UNUSED(time);

    if (isInsideShape(d->position) != m_inverted) {
        if (m_affectFlags & AffectPosition)
            d->position = m_targetPosition;

        if (m_affectFlags & AffectVelocity)
            d->velocity = m_targetVelocity;

        if (m_affectFlags & AffectRotation)
            d->rotation = m_targetRotation;

        if (m_affectFlags & AffectScale)
            d->scale = m_targetScale;

        if (m_affectFlags & AffectColor) {
            d->color.r = m_targetColor.red();
            d->color.g = m_targetColor.green();
            d->color.b = m_targetColor.blue();
            d->color.a = m_targetColor.alpha();
        }
    }
}

QVector3D ShapeAffector3D::worldToLocal(const QVector3D &worldPosition)
{
    return m_inverseTransform.map(worldPosition);
}

bool ShapeAffector3D::isInsideShape(const QVector3D &worldPosition)
{
    QVector3D localPosition = worldToLocal(worldPosition);
    switch (m_shapeType) {
    case Sphere:
        return isInsideSphere(localPosition);
    case Box:
        return isInsideBox(localPosition);
    case Cylinder:
        return isInsideCylinder(localPosition);
    default:
        return false;
    }
}

bool ShapeAffector3D::isInsideSphere(const QVector3D &localPosition)
{
    return localPosition.length() <= 50.0f;
}

bool ShapeAffector3D::isInsideBox(const QVector3D &localPosition)
{
    return qAbs(localPosition.x()) <= 50.0f &&
           qAbs(localPosition.y()) <= 50.0f &&
           qAbs(localPosition.z()) <= 50.0f;
}

bool ShapeAffector3D::isInsideCylinder(const QVector3D &localPosition)
{
    float radiusXZ = qSqrt(localPosition.x() * localPosition.x() +
                           localPosition.z() * localPosition.z());
    return radiusXZ <= 50.0f && qAbs(localPosition.y()) <= 50.0f;
}
