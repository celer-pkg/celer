// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/QMetaMethod>
#include "qquickgraphsbarsseries_p.h"
#include "utils_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

QQuickGraphsBar3DSeries::QQuickGraphsBar3DSeries(QObject *parent)
    : QBar3DSeries(parent)

{
    QObject::connect(this,
                     &QBar3DSeries::selectedBarChanged,
                     this,
                     &QQuickGraphsBar3DSeries::selectedBarChanged);
}

QQuickGraphsBar3DSeries::~QQuickGraphsBar3DSeries() {}

QQmlListProperty<QObject> QQuickGraphsBar3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this,
                                     this,
                                     &QQuickGraphsBar3DSeries::appendSeriesChildren,
                                     0,
                                     0,
                                     0);
}

void QQuickGraphsBar3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element)
{
    QBarDataProxy *proxy = qobject_cast<QBarDataProxy *>(element);
    if (proxy)
        reinterpret_cast<QQuickGraphsBar3DSeries *>(list->data)->setDataProxy(proxy);
}

void QQuickGraphsBar3DSeries::setSelectedBar(QPointF position)
{
    QBar3DSeries::setSelectedBar(position.toPoint());
}

QPointF QQuickGraphsBar3DSeries::selectedBar() const
{
    return QPointF(QBar3DSeries::selectedBar());
}

QPointF QQuickGraphsBar3DSeries::invalidSelectionPosition() const
{
    return QPointF(QBar3DSeries::invalidSelectionPosition());
}

void QQuickGraphsBar3DSeries::setBaseGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_baseGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
        return;
    }
    setGradientHelper(gradient, m_gradients.m_baseGradient, GradientType::Base);
    m_gradients.m_baseGradient = gradient;
    Q_EMIT baseGradientChanged(m_gradients.m_baseGradient);
}

QQuickGradient *QQuickGraphsBar3DSeries::baseGradient() const
{
    return m_gradients.m_baseGradient;
}

void QQuickGraphsBar3DSeries::setSingleHighlightGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_singleHighlightGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
        return;
    }
    setGradientHelper(gradient, m_gradients.m_singleHighlightGradient, GradientType::Single);
    m_gradients.m_singleHighlightGradient = gradient;
    Q_EMIT singleHighlightGradientChanged(m_gradients.m_singleHighlightGradient);
}

QQuickGradient *QQuickGraphsBar3DSeries::singleHighlightGradient() const
{
    return m_gradients.m_singleHighlightGradient;
}

void QQuickGraphsBar3DSeries::setMultiHighlightGradient(QQuickGradient *gradient)
{
    if (m_gradients.m_multiHighlightGradient == gradient) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gradient;
    }
    setGradientHelper(gradient, m_gradients.m_multiHighlightGradient, GradientType::Multi);
    m_gradients.m_multiHighlightGradient = gradient;
    Q_EMIT multiHighlightGradientChanged(m_gradients.m_multiHighlightGradient);
}

QQuickGradient *QQuickGraphsBar3DSeries::multiHighlightGradient() const
{
    return m_gradients.m_multiHighlightGradient;
}

QQmlListProperty<QQuickGraphsColor> QQuickGraphsBar3DSeries::rowColors()
{
    return QQmlListProperty<QQuickGraphsColor>(this,
                                               this,
                                               &QQuickGraphsBar3DSeries::appendRowColorsFunc,
                                               &QQuickGraphsBar3DSeries::countRowColorsFunc,
                                               &QQuickGraphsBar3DSeries::atRowColorsFunc,
                                               &QQuickGraphsBar3DSeries::clearRowColorsFunc);
}

void QQuickGraphsBar3DSeries::appendRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list,
                                                  QQuickGraphsColor *color)
{
    reinterpret_cast<QQuickGraphsBar3DSeries *>(list->data)->addColor(color);
}

qsizetype QQuickGraphsBar3DSeries::countRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list)
{
    return reinterpret_cast<QQuickGraphsBar3DSeries *>(list->data)->colorList().size();
}

QQuickGraphsColor *QQuickGraphsBar3DSeries::atRowColorsFunc(
    QQmlListProperty<QQuickGraphsColor> *list, qsizetype index)
{
    return reinterpret_cast<QQuickGraphsBar3DSeries *>(list->data)->colorList().at(index);
}

void QQuickGraphsBar3DSeries::clearRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list)
{
    reinterpret_cast<QQuickGraphsBar3DSeries *>(list->data)->clearColors();
}

void QQuickGraphsBar3DSeries::handleBaseGradientUpdate()
{
    if (!m_gradients.m_baseGradient)
        Utils::setSeriesGradient(this, m_gradients.m_baseGradient, GradientType::Base);
}

void QQuickGraphsBar3DSeries::handleSingleHighlightGradientUpdate()
{
    if (!m_gradients.m_singleHighlightGradient)
        Utils::setSeriesGradient(this, m_gradients.m_singleHighlightGradient, GradientType::Single);
}

void QQuickGraphsBar3DSeries::handleMultiHighlightGradientUpdate()
{
    if (!m_gradients.m_multiHighlightGradient)
        Utils::setSeriesGradient(this, m_gradients.m_multiHighlightGradient, GradientType::Multi);
}

void QQuickGraphsBar3DSeries::handleRowColorUpdate()
{
    qsizetype colorCount = m_rowColors.size();
    int changed = 0;

    QQuickGraphsColor *color = qobject_cast<QQuickGraphsColor *>(QObject::sender());
    for (int i = 0; i < colorCount; i++) {
        if (color == m_rowColors.at(i)) {
            changed = i;
            break;
        }
    }
    QList<QColor> list = QBar3DSeries::rowColors();
    list[changed] = m_rowColors.at(changed)->color();
    QBar3DSeries::setRowColors(list);
}

void QQuickGraphsBar3DSeries::addColor(QQuickGraphsColor *color)
{
    if (!color) {
        qCWarning(lcProperties3D, "%s invalid color used",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }
    clearDummyColors();
    m_rowColors.append(color);
    connect(color,
            &QQuickGraphsColor::colorChanged,
            this,
            &QQuickGraphsBar3DSeries::handleRowColorUpdate);
    QList<QColor> list = QBar3DSeries::rowColors();
    list.append(color->color());
    QBar3DSeries::setRowColors(list);
}

QList<QQuickGraphsColor *> QQuickGraphsBar3DSeries::colorList()
{
    if (m_rowColors.isEmpty()) {
        m_dummyColors = true;
        const QList<QColor> list = QBar3DSeries::rowColors();
        for (const QColor &item : list) {
            QQuickGraphsColor *color = new QQuickGraphsColor(this);
            color->setColor(item);
            m_rowColors.append(color);
            connect(color,
                    &QQuickGraphsColor::colorChanged,
                    this,
                    &QQuickGraphsBar3DSeries::handleRowColorUpdate);
        }
    }
    return m_rowColors;
}

void QQuickGraphsBar3DSeries::clearColors()
{
    clearDummyColors();
    for (const auto color : std::as_const(m_rowColors))
        disconnect(color, 0, this, 0);

    m_rowColors.clear();
    QBar3DSeries::setRowColors(QList<QColor>());
}

void QQuickGraphsBar3DSeries::clearDummyColors()
{
    if (m_dummyColors) {
        qDeleteAll(m_rowColors);
        m_rowColors.clear();
        m_dummyColors = false;
    }
}

void QQuickGraphsBar3DSeries::setGradientHelper(QQuickGradient *newGradient,
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
                             &QQuickGraphsBar3DSeries::handleBaseGradientUpdate);
            break;
        case GradientType::Single:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsBar3DSeries::handleSingleHighlightGradientUpdate);
            break;
        case GradientType::Multi:
            QObject::connect(memberGradient,
                             &QQuickGradient::updated,
                             this,
                             &QQuickGraphsBar3DSeries::handleMultiHighlightGradientUpdate);
            break;
        default:
            break;
        }
    }
}

QT_END_NAMESPACE

#include "moc_qquickgraphsbarsseries_p.cpp"
