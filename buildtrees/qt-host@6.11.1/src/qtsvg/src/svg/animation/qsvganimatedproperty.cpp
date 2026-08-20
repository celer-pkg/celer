// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvganimatedproperty_p.h"
#include <QtCore/qpoint.h>
#include <QtGui/qcolor.h>
#include <QtGui/qtransform.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qglobalstatic.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcSvgAnimatedProperty, "qt.svg.animation.properties")

typedef QHash<QString, QSvgAbstractAnimatedProperty::Type> AnimatableHashType;
Q_GLOBAL_STATIC(AnimatableHashType, animatableProperties)

static void initHash()
{
    animatableProperties->insert(QStringLiteral("fill"), QSvgAbstractAnimatedProperty::Color);
    animatableProperties->insert(QStringLiteral("fill-opacity"), QSvgAbstractAnimatedProperty::Float);
    animatableProperties->insert(QStringLiteral("stroke-opacity"), QSvgAbstractAnimatedProperty::Float);
    animatableProperties->insert(QStringLiteral("stroke"), QSvgAbstractAnimatedProperty::Color);
    animatableProperties->insert(QStringLiteral("opacity"), QSvgAbstractAnimatedProperty::Float);
    animatableProperties->insert(QStringLiteral("transform"), QSvgAbstractAnimatedProperty::Transform);
    animatableProperties->insert(QStringLiteral("offset-distance"), QSvgAbstractAnimatedProperty::Float);
}

static qreal q_lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
}

static QPointF pointInterpolator(QPointF v1, QPointF v2, qreal t)
{
    qreal x = q_lerp(v1.x(), v2.x(), t);
    qreal y = q_lerp(v1.y(), v2.y(), t);

    return QPointF(x, y);
}


QSvgAbstractAnimatedProperty::QSvgAbstractAnimatedProperty(const QString &name, Type type)
    : m_propertyName(name)
    , m_type(type)
{
}

QSvgAbstractAnimatedProperty::~QSvgAbstractAnimatedProperty()
{
}

void QSvgAbstractAnimatedProperty::setKeyFrames(const QList<qreal> &keyFrames)
{
    m_keyFrames = keyFrames;
}

void QSvgAbstractAnimatedProperty::appendKeyFrame(qreal keyFrame)
{
    m_keyFrames.append(keyFrame);
}

QList<qreal> QSvgAbstractAnimatedProperty::keyFrames() const
{
    return m_keyFrames;
}

void QSvgAbstractAnimatedProperty::setPropertyName(const QString &name)
{
    m_propertyName = name;
}

QStringView QSvgAbstractAnimatedProperty::propertyName() const
{
    return m_propertyName;
}

QSvgAbstractAnimatedProperty::Type QSvgAbstractAnimatedProperty::type() const
{
    return m_type;
}

QVariant QSvgAbstractAnimatedProperty::interpolatedValue() const
{
    return m_interpolatedValue;
}

QSvgAbstractAnimatedProperty *QSvgAbstractAnimatedProperty::createAnimatedProperty(const QString &name)
{
    if (animatableProperties->isEmpty())
        initHash();

    if (!animatableProperties->contains(name)) {
        qCDebug(lcSvgAnimatedProperty) << "Property : " << name << " is not animatable";
        return nullptr;
    }

    QSvgAbstractAnimatedProperty::Type type = animatableProperties->value(name);
    QSvgAbstractAnimatedProperty *prop = nullptr;

    switch (type) {
    case QSvgAbstractAnimatedProperty::Color:
        prop = new QSvgAnimatedPropertyColor(name);
        break;
    case QSvgAbstractAnimatedProperty::Transform:
        prop = new QSvgAnimatedPropertyTransform(name);
        break;
    case QSvgAbstractAnimatedProperty::Float:
        prop = new QSvgAnimatedPropertyFloat(name);
    default:
        break;
    }

    return prop;
}

QSvgAnimatedPropertyColor::QSvgAnimatedPropertyColor(const QString &name)
    : QSvgAbstractAnimatedProperty(name, QSvgAbstractAnimatedProperty::Color)
{
}

void QSvgAnimatedPropertyColor::setColors(const QList<QColor> &colors)
{
    m_colors = colors;
}

void QSvgAnimatedPropertyColor::appendColor(const QColor &color)
{
    m_colors.append(color);
}

QList<QColor> QSvgAnimatedPropertyColor::colors() const
{
    return m_colors;
}

void QSvgAnimatedPropertyColor::interpolate(uint index, qreal t) const
{
    QColor c1 = m_colors.at(index - 1);
    QColor c2 = m_colors.at(index);

    int alpha  = q_lerp(c1.alpha(), c2.alpha(), t);
    int red    = q_lerp(c1.red(), c2.red(), t);
    int green  = q_lerp(c1.green(), c2.green(), t);
    int blue   = q_lerp(c1.blue(), c2.blue(), t);

    m_interpolatedValue = QColor(red, green, blue, alpha);
}

QSvgAnimatedPropertyFloat::QSvgAnimatedPropertyFloat(const QString &name)
    : QSvgAbstractAnimatedProperty(name, QSvgAbstractAnimatedProperty::Float)
{
}

void QSvgAnimatedPropertyFloat::setValues(const QList<qreal> &values)
{
    m_values = values;
}

void QSvgAnimatedPropertyFloat::appendValue(const qreal value)
{
    m_values.append(value);
}

QList<qreal> QSvgAnimatedPropertyFloat::values() const
{
    return m_values;
}

void QSvgAnimatedPropertyFloat::interpolate(uint index, qreal t) const
{
    if (index >= (uint)m_keyFrames.size()) {
        qCWarning(lcSvgAnimatedProperty) << "Invalid index for key frames";
        return;
    }

    qreal float1 = m_values.at(index - 1);
    qreal float2 = m_values.at(index);

    m_interpolatedValue = q_lerp(float1, float2, t);
}

QSvgAnimatedPropertyTransform::QSvgAnimatedPropertyTransform(const QString &name)
    : QSvgAbstractAnimatedProperty(name, QSvgAbstractAnimatedProperty::Transform)
{

}

void QSvgAnimatedPropertyTransform::setTransformCount(quint32 count)
{
    m_transformCount = count;
}

quint32 QSvgAnimatedPropertyTransform::transformCount() const
{
    return m_transformCount;
}

void QSvgAnimatedPropertyTransform::appendComponents(const QList<TransformComponent> &components)
{
    m_components.append(components);
}

QList<QSvgAnimatedPropertyTransform::TransformComponent> QSvgAnimatedPropertyTransform::components() const
{
    return m_components;
}

// this function iterates over all TransformComponents in two consecutive
// key frames and interpolate between all TransformComponents. Moreover,
// it requires all key frames to have the same number of TransformComponents.
// This must be ensured by the parser itself, and it is handled in validateTransform
// function in qsvgcsshandler.cpp and in createAnimateTransformNode function
// in qsvghandler.cpp.
void QSvgAnimatedPropertyTransform::interpolate(uint index, qreal t) const
{
    if (index >= (uint)m_keyFrames.size()) {
        qCWarning(lcSvgAnimatedProperty) << "Invalid index for key frames";
        return;
    }

    if (!m_transformCount ||
        ((m_components.size() / qsizetype(m_transformCount)) != m_keyFrames.size())) {
        return;
    }

    QTransform transform = QTransform();

    qsizetype startIndex = (index - 1) * qsizetype(m_transformCount);
    qsizetype endIndex = index * qsizetype(m_transformCount);

    for (quint32 i = 0; i < m_transformCount; i++) {
        TransformComponent tc1 = m_components.at(startIndex + i);
        TransformComponent tc2 = m_components.at(endIndex + i);
        if (tc1.type == tc2.type) {
            if (tc1.type == TransformComponent::Translate) {
                QPointF t1 = QPointF(tc1.values.at(0), tc1.values.at(1));
                QPointF t2 = QPointF(tc2.values.at(0), tc2.values.at(1));
                QPointF tr = pointInterpolator(t1, t2, t);
                transform.translate(tr.x(), tr.y());
            } else if (tc1.type == TransformComponent::Scale) {
                QPointF s1 = QPointF(tc1.values.at(0), tc1.values.at(1));
                QPointF s2 = QPointF(tc2.values.at(0), tc2.values.at(1));
                QPointF sr = pointInterpolator(s1, s2, t);
                transform.scale(sr.x(), sr.y());
            } else if (tc1.type == TransformComponent::Rotate) {
                QPointF cor1 = QPointF(tc1.values.at(1), tc1.values.at(2));
                QPointF cor2 = QPointF(tc2.values.at(1), tc2.values.at(2));
                QPointF corResult = pointInterpolator(cor1, cor2, t);
                qreal angle1 = tc1.values.at(0);
                qreal angle2 = tc2.values.at(0);
                qreal angleResult = q_lerp(angle1, angle2, t);
                transform.translate(corResult.x(), corResult.y());
                transform.rotate(angleResult);
                transform.translate(-corResult.x(), -corResult.y());
            } else if (tc1.type == TransformComponent::Skew) {
                QPointF skew1 = QPointF(tc1.values.at(0), tc1.values.at(1));
                QPointF skew2 = QPointF(tc2.values.at(0), tc2.values.at(1));
                QPointF skewResult = pointInterpolator(skew1, skew2, t);
                transform.shear(qTan(qDegreesToRadians(skewResult.x())),
                                qTan(qDegreesToRadians(skewResult.y())));
            }
        }
    }

    m_interpolatedValue = transform;
}

QT_END_NAMESPACE
