// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QMetaMethod>
#include "qquickgraphsscatterseries_p.h"
#include "utils_p.h"

QT_BEGIN_NAMESPACE

QQuickGraphsScatter3DSeries::QQuickGraphsScatter3DSeries(QObject *parent)
    : QScatter3DSeries(parent)
{}

QQuickGraphsScatter3DSeries::~QQuickGraphsScatter3DSeries() {}

QQmlListProperty<QObject> QQuickGraphsScatter3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this,
                                     this,
                                     &QQuickGraphsScatter3DSeries::appendSeriesChildren,
                                     0,
                                     0,
                                     0);
}

void QQuickGraphsScatter3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list,
                                                       QObject *element)
{
    QScatterDataProxy *proxy = qobject_cast<QScatterDataProxy *>(element);
    if (proxy)
        reinterpret_cast<QQuickGraphsScatter3DSeries *>(list->data)->setDataProxy(proxy);
}

void QQuickGraphsScatter3DSeries::setBaseGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_baseGradient != gradient) {
        setGradientHelper(gradient, m_gradients.m_baseGradient, GradientType::Base);
        m_gradients.m_baseGradient = gradient;
        Q_EMIT baseGradientChanged(m_gradients.m_baseGradient);
    }
}

QQuickGradient *QQuickGraphsScatter3DSeries::baseGradient() const
{
    return m_gradients.m_baseGradient;
}

void QQuickGraphsScatter3DSeries::setSingleHighlightGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_singleHighlightGradient != gradient) {
        setGradientHelper(gradient, m_gradients.m_singleHighlightGradient, GradientType::Single);
        m_gradients.m_singleHighlightGradient = gradient;
        Q_EMIT singleHighlightGradientChanged(m_gradients.m_singleHighlightGradient);
    }
}

QQuickGradient *QQuickGraphsScatter3DSeries::singleHighlightGradient() const
{
    return m_gradients.m_singleHighlightGradient;
}

void QQuickGraphsScatter3DSeries::setMultiHighlightGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_multiHighlightGradient != gradient) {
        setGradientHelper(gradient, m_gradients.m_multiHighlightGradient, GradientType::Multi);
        m_gradients.m_multiHighlightGradient = gradient;
        Q_EMIT multiHighlightGradientChanged(m_gradients.m_multiHighlightGradient);
    }
}

QQuickGradient *QQuickGraphsScatter3DSeries::multiHighlightGradient() const
{
    return m_gradients.m_multiHighlightGradient;
}

qsizetype QQuickGraphsScatter3DSeries::invalidSelectionIndex() const
{
    return QScatter3DSeries::invalidSelectionIndex();
}

void QQuickGraphsScatter3DSeries::handleBaseGradientUpdate()
{
    if (!m_gradients.m_baseGradient)
        Utils::setSeriesGradient(this, m_gradients.m_baseGradient, GradientType::Base);
}

void QQuickGraphsScatter3DSeries::handleSingleHighlightGradientUpdate()
{
    if (!m_gradients.m_singleHighlightGradient)
        Utils::setSeriesGradient(this, m_gradients.m_singleHighlightGradient, GradientType::Single);
}

void QQuickGraphsScatter3DSeries::handleMultiHighlightGradientUpdate()
{
    if (!m_gradients.m_multiHighlightGradient)
        Utils::setSeriesGradient(this, m_gradients.m_multiHighlightGradient, GradientType::Multi);
}

void QQuickGraphsScatter3DSeries::setGradientHelper(QQuickGradient *newGradient,
                                                    QQuickGradient *memberGradient,
                                                    GradientType type)
{
    if (memberGradient)
        QObject::disconnect(memberGradient, 0, this, 0);
    Utils::setSeriesGradient(this, newGradient, type);
    memberGradient = newGradient;
    if (memberGradient) {
        switch (type) {
        case GradientType::Base:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsScatter3DSeries::handleBaseGradientUpdate);
            break;
        case GradientType::Single:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsScatter3DSeries::handleSingleHighlightGradientUpdate);
            break;
        case GradientType::Multi:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsScatter3DSeries::handleMultiHighlightGradientUpdate);
            break;
        default:
            break;
        }
    }
}

QT_END_NAMESPACE

#include "moc_qquickgraphsscatterseries_p.cpp"
