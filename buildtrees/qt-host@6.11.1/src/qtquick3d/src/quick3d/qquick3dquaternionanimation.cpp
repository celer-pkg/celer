// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dquaternionanimation_p.h"
#include <QtQuick/private/qquickanimation_p_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype QuaternionAnimation
    \inherits PropertyAnimation
    \inqmlmodule QtQuick3D
    \since 5.15

    \brief A PropertyAnimation for quaternions.

    A specialized \l{PropertyAnimation} that defines an animation between two
    \l{QQuaternion}{quaternions}.

    By default spherical linear interpolation is used. This can be changed to
    the faster but less accurate normalized linear interpolation by setting the
    \a type property.

    Instead of specifying quaternions directly in the \a from and \a to
    properties, it is also possible to provide euler angles in degrees in the
    \a fromXRotation, \a toXRotation, \a fromYRotation, \a toYRotation,
    \a fromZRotation, \a toZRotation properties.

    \note Avoid mixing the quaternion and euler angle-based properties. The
    from and to values are expected to be fully specified either via a
    quaternion or the three euler angles.

    \sa {Animation and Transitions in Qt Quick} QQuaternion QQuaternion::slerp() QQuaternion::nlerp()
*/

class QQuick3DQuaternionAnimationPrivate : public QQuickPropertyAnimationPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DQuaternionAnimation)

public:
    QQuick3DQuaternionAnimationPrivate() :
        type(QQuick3DQuaternionAnimation::Slerp)
    { }

    QQuick3DQuaternionAnimation::Type type;
    QVector3D anglesFrom;
    QVector3D anglesTo;
};


QVariant q_quaternionInterpolator(const QQuaternion &from, const QQuaternion &to, qreal progress)
{
    return QVariant::fromValue(QQuaternion::slerp(from, to, progress));
}

QVariant q_quaternionNlerpInterpolator(const QQuaternion &from, const QQuaternion &to, qreal progress)
{
    return QVariant::fromValue(QQuaternion::nlerp(from, to, progress));
}

QQuick3DQuaternionAnimation::QQuick3DQuaternionAnimation(QObject *parent)
    : QQuickPropertyAnimation(*(new QQuick3DQuaternionAnimationPrivate), parent)
{
    Q_D(QQuick3DQuaternionAnimation);
    d->interpolatorType = qMetaTypeId<QQuaternion>();
    d->defaultToInterpolatorType = true;
    d->interpolator = QVariantAnimationPrivate::getInterpolator(d->interpolatorType);
}

/*!
    \qmlproperty quaternion QtQuick3D::QuaternionAnimation::from

    This property holds the starting value for the animation.

*/

QQuaternion QQuick3DQuaternionAnimation::from() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->from.value<QQuaternion>();
}

void QQuick3DQuaternionAnimation::setFrom(const QQuaternion &f)
{
    QQuickPropertyAnimation::setFrom(QVariant::fromValue(f));
}

/*!
    \qmlproperty quaternion QtQuick3D::QuaternionAnimation::to

    This property holds the ending value for the animation.

*/

QQuaternion QQuick3DQuaternionAnimation::to() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->to.value<QQuaternion>();
}

void QQuick3DQuaternionAnimation::setTo(const QQuaternion &t)
{
    QQuickPropertyAnimation::setTo(QVariant::fromValue(t));
}

/*!
    \qmlproperty enumeration QtQuick3D::QuaternionAnimation::type

    This property defines the interpolation mode.

    \value QuaternionAnimation.Slerp Spherical linear interpolation.
    \value QuaternionAnimation.Nlerp Normalized linear interpolation.

*/

QQuick3DQuaternionAnimation::Type QQuick3DQuaternionAnimation::type() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->type;
}

void QQuick3DQuaternionAnimation::setType(Type type)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->type == type)
        return;

    d->type = type;
    switch (type) {
    case Nlerp:
        d->interpolator = reinterpret_cast<QVariantAnimation::Interpolator>(reinterpret_cast<void(*)()>(&q_quaternionNlerpInterpolator));
        break;
    case Slerp:
    default:
        d->interpolator = QVariantAnimationPrivate::getInterpolator(d->interpolatorType);
        break;
    }

    emit typeChanged(type);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::fromXRotation

    This property holds the starting value of the animation for the X axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::fromXRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesFrom.x();
}

void QQuick3DQuaternionAnimation::setFromXRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesFrom.x() == f)
        return;
    d->anglesFrom.setX(f);
    setFrom(QQuaternion::fromEulerAngles(d->anglesFrom));
    emit fromXRotationChanged(f);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::fromYRotation

    This property holds the starting value of the animation for the Y axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::fromYRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesFrom.y();
}

void QQuick3DQuaternionAnimation::setFromYRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesFrom.y() == f)
        return;
    d->anglesFrom.setY(f);
    setFrom(QQuaternion::fromEulerAngles(d->anglesFrom));
    emit fromYRotationChanged(f);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::fromZRotation

    This property holds the starting value of the animation for the Z axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::fromZRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesFrom.z();
}

void QQuick3DQuaternionAnimation::setFromZRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesFrom.z() == f)
        return;
    d->anglesFrom.setZ(f);
    setFrom(QQuaternion::fromEulerAngles(d->anglesFrom));
    emit fromZRotationChanged(f);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::toXRotation

    This property holds the ending value of the animation for the X axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::toXRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesTo.x();
}

void QQuick3DQuaternionAnimation::setToXRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesTo.x() == f)
        return;
    d->anglesTo.setX(f);
    setTo(QQuaternion::fromEulerAngles(d->anglesTo));
    emit toXRotationChanged(f);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::toYRotation

    This property holds the ending value of the animation for the Y axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::toYRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesTo.y();
}

void QQuick3DQuaternionAnimation::setToYRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesTo.y() == f)
        return;
    d->anglesTo.setY(f);
    setTo(QQuaternion::fromEulerAngles(d->anglesTo));
    emit toYRotationChanged(f);
}

/*!
    \qmlproperty real QtQuick3D::QuaternionAnimation::toZRotation

    This property holds the ending value of the animation for the Z axis as
    an euler angle in degrees.

*/

float QQuick3DQuaternionAnimation::toZRotation() const
{
    Q_D(const QQuick3DQuaternionAnimation);
    return d->anglesTo.z();
}

void QQuick3DQuaternionAnimation::setToZRotation(float f)
{
    Q_D(QQuick3DQuaternionAnimation);
    if (d->anglesTo.z() == f)
        return;
    d->anglesTo.setZ(f);
    setTo(QQuaternion::fromEulerAngles(d->anglesTo));
    emit toZRotationChanged(f);
}

QT_END_NAMESPACE
