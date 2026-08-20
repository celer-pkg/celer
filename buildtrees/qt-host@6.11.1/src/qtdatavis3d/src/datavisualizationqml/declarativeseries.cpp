// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "declarativeseries_p.h"
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

static void setSeriesGradient(QAbstract3DSeries *series, const ColorGradient &gradient,
                              GradientType type)
{
    QLinearGradient newGradient;
    QGradientStops stops;
    QList<ColorGradientStop *> qmlstops = gradient.m_stops;

    // Get sorted gradient stops
    for (int i = 0; i < qmlstops.size(); i++) {
        int j = 0;
        while (j < stops.size() && stops.at(j).first < qmlstops[i]->position())
            j++;
        stops.insert(j, QGradientStop(qmlstops.at(i)->position(), qmlstops.at(i)->color()));
    }

    newGradient.setStops(stops);
    switch (type) {
    case GradientTypeBase:
        series->setBaseGradient(newGradient);
        break;
    case GradientTypeSingle:
        series->setSingleHighlightGradient(newGradient);
        break;
    case GradientTypeMulti:
        series->setMultiHighlightGradient(newGradient);
        break;
    default: // Never goes here
        break;
    }
}

static void connectSeriesGradient(QAbstract3DSeries *series, ColorGradient *newGradient,
                                  GradientType type, ColorGradient **memberGradient)
{
    // connect new / disconnect old
    if (newGradient != *memberGradient) {
        if (*memberGradient)
            QObject::disconnect(*memberGradient, 0, series, 0);

        *memberGradient = newGradient;

        int updatedIndex = newGradient->metaObject()->indexOfSignal("updated()");
        QMetaMethod updateFunction = newGradient->metaObject()->method(updatedIndex);
        int handleIndex = -1;
        switch (type) {
        case GradientTypeBase:
            handleIndex = series->metaObject()->indexOfSlot("handleBaseGradientUpdate()");
            break;
        case GradientTypeSingle:
            handleIndex = series->metaObject()->indexOfSlot("handleSingleHighlightGradientUpdate()");
            break;
        case GradientTypeMulti:
            handleIndex = series->metaObject()->indexOfSlot("handleMultiHighlightGradientUpdate()");
            break;
        default: // Never goes here
            break;
        }
        QMetaMethod handleFunction = series->metaObject()->method(handleIndex);

        if (*memberGradient)
            QObject::connect(*memberGradient, updateFunction, series, handleFunction);
    }

    if (*memberGradient)
        setSeriesGradient(series, **memberGradient, type);
}

DeclarativeBar3DSeries::DeclarativeBar3DSeries(QObject *parent)
    : QBar3DSeries(parent),
      m_baseGradient(0),
      m_singleHighlightGradient(0),
      m_multiHighlightGradient(0),
      m_dummyColors(false)
{
    QObject::connect(this, &QBar3DSeries::selectedBarChanged, this,
                     &DeclarativeBar3DSeries::selectedBarChanged);
}

DeclarativeBar3DSeries::~DeclarativeBar3DSeries()
{
}

QQmlListProperty<QObject> DeclarativeBar3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this, this, &DeclarativeBar3DSeries::appendSeriesChildren
                                     , 0, 0, 0);
}

void DeclarativeBar3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element)
{
    QBarDataProxy *proxy = qobject_cast<QBarDataProxy *>(element);
    if (proxy)
        reinterpret_cast<DeclarativeBar3DSeries *>(list->data)->setDataProxy(proxy);
}

void DeclarativeBar3DSeries::setSelectedBar(const QPointF &position)
{
    QBar3DSeries::setSelectedBar(position.toPoint());
}

QPointF DeclarativeBar3DSeries::selectedBar() const
{
    return QPointF(QBar3DSeries::selectedBar());
}

QPointF DeclarativeBar3DSeries::invalidSelectionPosition() const
{
    return QPointF(QBar3DSeries::invalidSelectionPosition());
}

void DeclarativeBar3DSeries::setBaseGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeBase, &m_baseGradient);
}

ColorGradient *DeclarativeBar3DSeries::baseGradient() const
{
    return m_baseGradient;
}

void DeclarativeBar3DSeries::setSingleHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeSingle, &m_singleHighlightGradient);
}

ColorGradient *DeclarativeBar3DSeries::singleHighlightGradient() const
{
    return m_singleHighlightGradient;
}

void DeclarativeBar3DSeries::setMultiHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeMulti, &m_multiHighlightGradient);
}

ColorGradient *DeclarativeBar3DSeries::multiHighlightGradient() const
{
    return m_multiHighlightGradient;
}

QQmlListProperty<DeclarativeColor> DeclarativeBar3DSeries::rowColors()
{
    return QQmlListProperty<DeclarativeColor>(this, this,
                                              &DeclarativeBar3DSeries::appendRowColorsFunc,
                                              &DeclarativeBar3DSeries::countRowColorsFunc,
                                              &DeclarativeBar3DSeries::atRowColorsFunc,
                                              &DeclarativeBar3DSeries::clearRowColorsFunc);
}

void DeclarativeBar3DSeries::appendRowColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                                 DeclarativeColor *color)
{
    reinterpret_cast<DeclarativeBar3DSeries *>(list->data)->addColor(color);
}

qsizetype DeclarativeBar3DSeries::countRowColorsFunc(QQmlListProperty<DeclarativeColor> *list)
{
    return reinterpret_cast<DeclarativeBar3DSeries *>(list->data)->colorList().size();
}

DeclarativeColor *DeclarativeBar3DSeries::atRowColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                                          qsizetype index)
{
    return reinterpret_cast<DeclarativeBar3DSeries *>(list->data)->colorList().at(index);
}

void DeclarativeBar3DSeries::clearRowColorsFunc(QQmlListProperty<DeclarativeColor> *list)
{
    reinterpret_cast<DeclarativeBar3DSeries *>(list->data)->clearColors();
}

void DeclarativeBar3DSeries::handleBaseGradientUpdate()
{
    if (m_baseGradient)
        setSeriesGradient(this, *m_baseGradient, GradientTypeBase);
}

void DeclarativeBar3DSeries::handleSingleHighlightGradientUpdate()
{
    if (m_singleHighlightGradient)
        setSeriesGradient(this, *m_singleHighlightGradient, GradientTypeSingle);
}

void DeclarativeBar3DSeries::handleMultiHighlightGradientUpdate()
{
    if (m_multiHighlightGradient)
        setSeriesGradient(this, *m_multiHighlightGradient, GradientTypeMulti);
}

void DeclarativeBar3DSeries::handleRowColorUpdate()
{
    int colorCount = m_rowColors.size();
    int changed = 0;

    DeclarativeColor *color = qobject_cast<DeclarativeColor*>(QObject::sender());
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

void DeclarativeBar3DSeries::addColor(DeclarativeColor *color)
{
    if (!color) {
        qWarning("Color is invalid, use ThemeColor");
        return;
    }
    clearDummyColors();
    m_rowColors.append(color);
    connect(color, &DeclarativeColor::colorChanged, this,
            &DeclarativeBar3DSeries::handleRowColorUpdate);
    QList<QColor> list = QBar3DSeries::rowColors();
    list.append(color->color());
    QBar3DSeries::setRowColors(list);
}

QList<DeclarativeColor *> DeclarativeBar3DSeries::colorList()
{
    if (m_rowColors.isEmpty()) {
        m_dummyColors = true;
        const QList<QColor> list = QBar3DSeries::rowColors();
        for (const QColor &item : list) {
            DeclarativeColor *color = new DeclarativeColor(this);
            color->setColor(item);
            m_rowColors.append(color);
            connect(color, &DeclarativeColor::colorChanged, this,
                    &DeclarativeBar3DSeries::handleRowColorUpdate);
        }
    }
    return m_rowColors;
}

void DeclarativeBar3DSeries::clearColors()
{
    clearDummyColors();
    for (const auto color : std::as_const(m_rowColors))
        disconnect(color, 0, this, 0);

    m_rowColors.clear();
    QBar3DSeries::setRowColors(QList<QColor>());
}

void DeclarativeBar3DSeries::clearDummyColors()
{
    if (m_dummyColors) {
        qDeleteAll(m_rowColors);
        m_rowColors.clear();
        m_dummyColors = false;
    }
}

DeclarativeScatter3DSeries::DeclarativeScatter3DSeries(QObject *parent)
    : QScatter3DSeries(parent),
      m_baseGradient(0),
      m_singleHighlightGradient(0),
      m_multiHighlightGradient(0)
{
}

DeclarativeScatter3DSeries::~DeclarativeScatter3DSeries()
{
}

QQmlListProperty<QObject> DeclarativeScatter3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this, this, &DeclarativeScatter3DSeries::appendSeriesChildren
                                     , 0, 0, 0);
}

void DeclarativeScatter3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list,
                                                      QObject *element)
{
    QScatterDataProxy *proxy = qobject_cast<QScatterDataProxy *>(element);
    if (proxy)
        reinterpret_cast<DeclarativeScatter3DSeries *>(list->data)->setDataProxy(proxy);
}

void DeclarativeScatter3DSeries::setBaseGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeBase, &m_baseGradient);
}

ColorGradient *DeclarativeScatter3DSeries::baseGradient() const
{
    return m_baseGradient;
}

void DeclarativeScatter3DSeries::setSingleHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeSingle, &m_singleHighlightGradient);
}

ColorGradient *DeclarativeScatter3DSeries::singleHighlightGradient() const
{
    return m_singleHighlightGradient;
}

void DeclarativeScatter3DSeries::setMultiHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeMulti, &m_multiHighlightGradient);
}

ColorGradient *DeclarativeScatter3DSeries::multiHighlightGradient() const
{
    return m_multiHighlightGradient;
}

int DeclarativeScatter3DSeries::invalidSelectionIndex() const
{
    return QScatter3DSeries::invalidSelectionIndex();
}

void DeclarativeScatter3DSeries::handleBaseGradientUpdate()
{
    if (m_baseGradient)
        setSeriesGradient(this, *m_baseGradient, GradientTypeBase);
}

void DeclarativeScatter3DSeries::handleSingleHighlightGradientUpdate()
{
    if (m_singleHighlightGradient)
        setSeriesGradient(this, *m_singleHighlightGradient, GradientTypeSingle);
}

void DeclarativeScatter3DSeries::handleMultiHighlightGradientUpdate()
{
    if (m_multiHighlightGradient)
        setSeriesGradient(this, *m_multiHighlightGradient, GradientTypeMulti);
}

DeclarativeSurface3DSeries::DeclarativeSurface3DSeries(QObject *parent)
    : QSurface3DSeries(parent),
      m_baseGradient(0),
      m_singleHighlightGradient(0),
      m_multiHighlightGradient(0)
{
    QObject::connect(this, &QSurface3DSeries::selectedPointChanged, this,
                     &DeclarativeSurface3DSeries::selectedPointChanged);
}

DeclarativeSurface3DSeries::~DeclarativeSurface3DSeries()
{
}

void DeclarativeSurface3DSeries::setSelectedPoint(const QPointF &position)
{
    QSurface3DSeries::setSelectedPoint(position.toPoint());
}

QPointF DeclarativeSurface3DSeries::selectedPoint() const
{
    return QPointF(QSurface3DSeries::selectedPoint());
}

QPointF DeclarativeSurface3DSeries::invalidSelectionPosition() const
{
    return QPointF(QSurface3DSeries::invalidSelectionPosition());
}

QQmlListProperty<QObject> DeclarativeSurface3DSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this, this, &DeclarativeSurface3DSeries::appendSeriesChildren
                                     , 0, 0, 0);
}

void DeclarativeSurface3DSeries::appendSeriesChildren(QQmlListProperty<QObject> *list,
                                                      QObject *element)
{
    QSurfaceDataProxy *proxy = qobject_cast<QSurfaceDataProxy *>(element);
    if (proxy)
        reinterpret_cast<DeclarativeSurface3DSeries *>(list->data)->setDataProxy(proxy);
}

void DeclarativeSurface3DSeries::setBaseGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeBase, &m_baseGradient);
}

ColorGradient *DeclarativeSurface3DSeries::baseGradient() const
{
    return m_baseGradient;
}

void DeclarativeSurface3DSeries::setSingleHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeSingle, &m_singleHighlightGradient);
}

ColorGradient *DeclarativeSurface3DSeries::singleHighlightGradient() const
{
    return m_singleHighlightGradient;
}

void DeclarativeSurface3DSeries::setMultiHighlightGradient(ColorGradient *gradient)
{
    connectSeriesGradient(this, gradient, GradientTypeMulti, &m_multiHighlightGradient);
}

ColorGradient *DeclarativeSurface3DSeries::multiHighlightGradient() const
{
    return m_multiHighlightGradient;
}

void DeclarativeSurface3DSeries::handleBaseGradientUpdate()
{
    if (m_baseGradient)
        setSeriesGradient(this, *m_baseGradient, GradientTypeBase);
}

void DeclarativeSurface3DSeries::handleSingleHighlightGradientUpdate()
{
    if (m_singleHighlightGradient)
        setSeriesGradient(this, *m_singleHighlightGradient, GradientTypeSingle);
}

void DeclarativeSurface3DSeries::handleMultiHighlightGradientUpdate()
{
    if (m_multiHighlightGradient)
        setSeriesGradient(this, *m_multiHighlightGradient, GradientTypeMulti);
}

QT_END_NAMESPACE
