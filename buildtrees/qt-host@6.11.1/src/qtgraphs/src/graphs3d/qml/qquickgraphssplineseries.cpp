// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QMetaMethod>
#include "qquickgraphssplineseries_p.h"
#include "utils_p.h"

QT_BEGIN_NAMESPACE

QQuickGraphsSpline3DSeries::QQuickGraphsSpline3DSeries(QObject *parent)
    : QSpline3DSeries(parent)
{}

QQuickGraphsSpline3DSeries::~QQuickGraphsSpline3DSeries() {}

QQmlListProperty<QObject> QQuickGraphsSpline3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this,
                                     this,
                                     &QQuickGraphsSpline3DSeries::appendSeriesChildren,
                                     0,
                                     0,
                                     0);
}

void QQuickGraphsSpline3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list,
                                                      QObject *element)
{
    QScatterDataProxy *proxy = qobject_cast<QScatterDataProxy *>(element);
    if (proxy)
        reinterpret_cast<QQuickGraphsSpline3DSeries *>(list->data)->setDataProxy(proxy);
}

void QQuickGraphsSpline3DSeries::setBaseGradient(QQuickGradient *gradient)
{
    if (m_baseGradient != gradient) {
        setGradientHelper(gradient, m_baseGradient, GradientType::Base);
        m_baseGradient = gradient;
        Q_EMIT baseGradientChanged(m_baseGradient);
    }
}

QQuickGradient *QQuickGraphsSpline3DSeries::baseGradient() const
{
    return m_baseGradient;
}

void QQuickGraphsSpline3DSeries::setSingleHighlightGradient(QQuickGradient *gradient)
{
    if (m_singleHighlightGradient != gradient) {
        setGradientHelper(gradient, m_singleHighlightGradient, GradientType::Single);
        m_singleHighlightGradient = gradient;
        Q_EMIT singleHighlightGradientChanged(m_singleHighlightGradient);
    }
}

QQuickGradient *QQuickGraphsSpline3DSeries::singleHighlightGradient() const
{
    return m_singleHighlightGradient;
}

void QQuickGraphsSpline3DSeries::setMultiHighlightGradient(QQuickGradient *gradient)
{
    if (m_multiHighlightGradient != gradient) {
        setGradientHelper(gradient, m_multiHighlightGradient, GradientType::Multi);
        m_multiHighlightGradient = gradient;
        Q_EMIT multiHighlightGradientChanged(m_multiHighlightGradient);
    }
}

QQuickGradient *QQuickGraphsSpline3DSeries::multiHighlightGradient() const
{
    return m_multiHighlightGradient;
}

int QQuickGraphsSpline3DSeries::invalidSelectionIndex() const
{
    return QSpline3DSeries::invalidSelectionIndex();
}

void QQuickGraphsSpline3DSeries::handleBaseGradientUpdate()
{
    if (!m_baseGradient)
        Utils::setSeriesGradient(this, m_baseGradient, GradientType::Base);
}

void QQuickGraphsSpline3DSeries::handleSingleHighlightGradientUpdate()
{
    if (!m_singleHighlightGradient)
        Utils::setSeriesGradient(this, m_singleHighlightGradient, GradientType::Single);
}

void QQuickGraphsSpline3DSeries::handleMultiHighlightGradientUpdate()
{
    if (!m_multiHighlightGradient)
        Utils::setSeriesGradient(this, m_multiHighlightGradient, GradientType::Multi);
}

void QQuickGraphsSpline3DSeries::setGradientHelper(QQuickGradient *newGradient,
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
                             &QQuickGraphsSpline3DSeries::handleBaseGradientUpdate);
            break;
        case GradientType::Single:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsSpline3DSeries::handleSingleHighlightGradientUpdate);
            break;
        case GradientType::Multi:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsSpline3DSeries::handleMultiHighlightGradientUpdate);
            break;
        default:
            break;
        }
    }
}

QT_END_NAMESPACE

#include "moc_qquickgraphssplineseries_p.cpp"
