// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qblendanimationnode_p.h"

#include <QVariant>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QColor>
#include <QRect>
#include <QRectF>

QT_BEGIN_NAMESPACE

/*!
    \qmltype BlendAnimationNode
    \inherits BlendTreeNode
    \nativetype QBlendAnimationNode
    \inqmlmodule QtQuick.Timeline.BlendTrees
    \ingroup qtqmltypes

    \brief A blend tree node that blends between two animation sources.

    BlendAnimationNode is a blend tree node that blends between two animation
    sources based on a weight value. The weight value can be animated to
    dynamically blend between the two animation sources.
*/

/*!
    \qmlproperty BlendTreeNode BlendAnimationNode::source1

    This property holds the first animation source.
*/

/*!
    \qmlproperty BlendTreeNode BlendAnimationNode::source2

    This property holds the second animation source.
*/

/*!
    \qmlproperty real BlendAnimationNode::weight

    This property holds the weight value used to blend between the two animation
    sources.
    The weight value determines how much of the first animation source is blended
    with the second animation source. A weight value of \c 0.0 means the first
    animation source is used exclusively, a weight value of \c 1.0 means the
    second animation source is used exclusively, and a weight value of \c 0.5 means
    both animation sources are blended equally.  The default value is \c 0.5.
*/

QBlendAnimationNode::QBlendAnimationNode(QObject *parent)
    : QBlendTreeNode(parent)
{
    connect(this,
            &QBlendAnimationNode::weightChanged,
            this,
            &QBlendAnimationNode::handleInputFrameDataChanged);
}

QBlendTreeNode *QBlendAnimationNode::source1() const
{
    return m_source1;
}

void QBlendAnimationNode::setSource1(QBlendTreeNode *newSource1)
{
    if (m_source1 == newSource1)
        return;

    if (m_source1) {
        disconnect(m_source1OutputConnection);
        disconnect(m_source1DestroyedConnection);
    }

    m_source1 = newSource1;

    if (m_source1) {
        m_source1OutputConnection = connect(m_source1,
                                            &QBlendTreeNode::frameDataChanged,
                                            this,
                                            &QBlendAnimationNode::handleInputFrameDataChanged);
        m_source1DestroyedConnection = connect(m_source1,
                                               &QObject::destroyed,
                                               this,
                                               [this] { setSource1(nullptr);});
    }
    Q_EMIT source1Changed();
}

QBlendTreeNode *QBlendAnimationNode::source2() const
{
    return m_source2;
}

void QBlendAnimationNode::setSource2(QBlendTreeNode *newSource2)
{
    if (m_source2 == newSource2)
        return;

    if (m_source2) {
        disconnect(m_source2OutputConnection);
        disconnect(m_source2DestroyedConnection);
    }

    m_source2 = newSource2;

    if (m_source2) {
        m_source2OutputConnection = connect(m_source2,
                                            &QBlendTreeNode::frameDataChanged,
                                            this,
                                            &QBlendAnimationNode::handleInputFrameDataChanged);
        m_source2DestroyedConnection = connect(m_source2,
                                               &QObject::destroyed,
                                               this,
                                               [this] { setSource2(nullptr);});
    }

    Q_EMIT source2Changed();
}

qreal QBlendAnimationNode::weight() const
{
    return m_weight;
}

void QBlendAnimationNode::setWeight(qreal newWeight)
{
    if (qFuzzyCompare(m_weight, newWeight))
        return;
    m_weight = newWeight;
    Q_EMIT weightChanged();
}

static QVariant lerp(const QVariant &first, const QVariant &second, float weight)
{
    // Don't bother with weights if there is no data (for now)
    if (first.isNull())
        return second;
    else if (second.isNull())
        return first;

    const QMetaType type = first.metaType();
    switch (type.id()) {
    case QMetaType::Bool:
        return QVariant((1.0f - weight) * first.toBool() + weight * second.toBool() >= 0.5f);
    case QMetaType::Int:
        return QVariant((1.0f - weight) * first.toInt() + weight * second.toInt());
    case QMetaType::Float:
        return QVariant((1.0f - weight) * first.toFloat() + weight * second.toFloat());
    case QMetaType::Double:
        return QVariant((1.0 - weight) * first.toDouble() + weight * second.toDouble());
    case QMetaType::QVector2D: {
        QVector2D firstVec = first.value<QVector2D>();
        QVector2D secondVec = second.value<QVector2D>();
        return QVariant::fromValue<QVector2D>(firstVec * (1.0f - weight) + secondVec * weight);
    }
    case QMetaType::QVector3D: {
        QVector3D firstVec = first.value<QVector3D>();
        QVector3D secondVec = second.value<QVector3D>();
        return QVariant::fromValue<QVector3D>(firstVec * (1.0f - weight) + secondVec * weight);
    }
    case QMetaType::QVector4D: {
        QVector4D firstVec = first.value<QVector4D>();
        QVector4D secondVec = second.value<QVector4D>();
        return QVariant::fromValue<QVector4D>(firstVec * (1.0f - weight) + secondVec * weight);
    }
    case QMetaType::QQuaternion: {
        QQuaternion firstQuat = first.value<QQuaternion>();
        QQuaternion secondQuat = second.value<QQuaternion>();
        return QVariant::fromValue<QQuaternion>(QQuaternion::nlerp(firstQuat, secondQuat, weight));
    }
    case QMetaType::QColor: {
        QColor firstColor = first.value<QColor>();
        QColor secondColor = second.value<QColor>();
        int r = (1.0f - weight) * firstColor.red() + weight * secondColor.red();
        int g = (1.0f - weight) * firstColor.green() + weight * secondColor.green();
        int b = (1.0f - weight) * firstColor.blue() + weight * secondColor.blue();
        int a = (1.0f - weight) * firstColor.alpha() + weight * secondColor.alpha();
        return QVariant::fromValue<QColor>(QColor(r, g, b, a));
    }
    case QMetaType::QRect: {
        QRect firstRect = first.value<QRect>();
        QRect secondRect = second.value<QRect>();
        int x = (1.0f - weight) * firstRect.x() + weight * secondRect.x();
        int y = (1.0f - weight) * firstRect.y() + weight * secondRect.y();
        int width = (1.0f - weight) * firstRect.width() + weight * secondRect.width();
        int height = (1.0f - weight) * firstRect.height() + weight * secondRect.height();
        return QVariant::fromValue<QRect>(QRect(x, y, width, height));
    }
    case QMetaType::QRectF: {
        QRectF firstRectF = first.value<QRectF>();
        QRectF secondRectF = second.value<QRectF>();
        qreal x = (1.0 - weight) * firstRectF.x() + weight * secondRectF.x();
        qreal y = (1.0 - weight) * firstRectF.y() + weight * secondRectF.y();
        qreal width = (1.0 - weight) * firstRectF.width() + weight * secondRectF.width();
        qreal height = (1.0 - weight) * firstRectF.height() + weight * secondRectF.height();
        return QVariant::fromValue<QRectF>(QRectF(x, y, width, height));
    }
    default:
        // Unsupported type, return an invalid QVariant
        return QVariant();
    }

}

void QBlendAnimationNode::handleInputFrameDataChanged()
{
    const QHash<QQmlProperty, QVariant> &frameData1 = m_source1 ? m_source1->frameData() : QHash<QQmlProperty, QVariant>();
    const QHash<QQmlProperty, QVariant> &frameData2 = m_source2 ? m_source2->frameData() : QHash<QQmlProperty, QVariant>();

    // Do the LERP blending here
    if (m_weight <= 0.0) {
        // all source1
        m_frameData = frameData1;
    } else if (m_weight >= 1.0) {
        // all source2
        m_frameData = frameData2;
    } else {
        // is a mix
        QHash<QQmlProperty, QPair<QVariant, QVariant>> allData;
        const auto &keys1 = frameData1.keys();
        for (const auto &property : keys1)
            allData.insert(property, QPair<QVariant, QVariant>(frameData1[property], QVariant()));
        const auto &keys2 = frameData2.keys();
        for (const auto &property : keys2) {
            // first check if property is already in all data, and if so modify the pair to include this value
            if (allData.contains(property)) {
                allData[property].second = frameData2[property];
            } else {
                allData.insert(property, QPair<QVariant, QVariant>(QVariant(), frameData2[property]));
            }
        }

        QHash<QQmlProperty, QVariant> newFrameData;

        const auto &keys = allData.keys();
        for (const auto &property : keys) {
            const auto &dataPair = allData[property];
            newFrameData.insert(property, lerp(dataPair.first, dataPair.second, m_weight));
        }
        m_frameData = newFrameData;
    }

    Q_EMIT frameDataChanged();
}

QT_END_NAMESPACE
