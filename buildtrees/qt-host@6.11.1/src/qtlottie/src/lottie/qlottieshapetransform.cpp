// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieshapetransform_p.h"

#include <QJsonObject>
#include <QtMath>

#include "qlottieconstants_p.h"
#include "qlottiebasictransform_p.h"

QLottieShapeTransform::QLottieShapeTransform(const QLottieShapeTransform &other)
    : QLottieBasicTransform(other)
{
    m_skew = other.m_skew;
    m_skewAxis = other.m_skewAxis;
    m_shearX = other.m_shearX;
    m_shearY = other.m_shearY;
    m_shearAngle = other.m_shearAngle;
}

QLottieShapeTransform::QLottieShapeTransform(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieShapeTransform::clone() const
{
    return new QLottieShapeTransform(*this);
}

int QLottieShapeTransform::parse(const QJsonObject &definition)
{
    if (QLottieBasicTransform::parse(definition) < 0)
        return -1;

    qCDebug(lcLottieQtLottieParser) << "QLottieShapeTransform::parse():" << QLottieShape::name();

    if (definition.contains("sk"_L1)) {
        QJsonObject skew = definition.value("sk"_L1).toObject();
        skew = resolveExpression(skew);
        m_skew.construct(skew);
    }

    if (definition.contains("sa"_L1)) {
        QJsonObject skewAxis = definition.value("sa"_L1).toObject();
        skewAxis = resolveExpression(skewAxis);
        m_skewAxis.construct(skewAxis);
    }

    return 0;
}

void QLottieShapeTransform::updateProperties(int frame)
{
    QLottieBasicTransform::updateProperties(frame);

    m_skew.update(frame);
    m_skewAxis.update(frame);

    double rads = qDegreesToRadians(m_skewAxis.value());
    m_shearX = qCos(rads);
    m_shearY = qSin(rads);
    double tan = qDegreesToRadians(-m_skew.value());
    m_shearAngle = qTan(tan);
}

void QLottieShapeTransform::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

qreal QLottieShapeTransform::skew() const
{
    return m_skew.value();
}

qreal QLottieShapeTransform::skewAxis() const
{
    return m_skewAxis.value();
}

qreal QLottieShapeTransform::shearX() const
{
    return m_shearX;
}

qreal QLottieShapeTransform::shearY() const
{
    return m_shearY;
}

qreal QLottieShapeTransform::shearAngle() const
{
    return m_shearAngle;
}
