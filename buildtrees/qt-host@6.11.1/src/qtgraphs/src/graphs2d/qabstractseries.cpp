// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/qabstractseries.h"
#include <QtGraphs/qabstractseries.h>
#include <private/qabstractseries_p.h>
#include <private/qgraphsview_p.h>
#include <private/qabstractaxis_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QAbstractSeries class is a base class for all Qt Graphs for 2D series.

    Usually, the series type specific inherited classes are used instead of the base class.

    \sa QLineSeries, QSplineSeries, QScatterSeries, QBarSeries, QXYSeries
*/
/*!
    \qmltype AbstractSeries
    \nativetype QAbstractSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \brief Base type for all Qt Graph series types.

    This type cannot be instantiated directly. Instead, one of the following derived types
    should be used to create a series: LineSeries, SplineSeries, BarSeries, or ScatterSeries.
*/

/*!
    \class QLegendData
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QLegendData struct contains information to display on a sets
    legend marker.

    The information needed to make a visual association between a set and a
    marker include properties such as color, border color, and a name of a set.

    \sa QLineSeries, QSplineSeries, QScatterSeries, QBarSeries, QXYSeries
*/

/*!
    \qmlvaluetype legendData
    \nativetype QLegendData
    \inqmlmodule QtGraphs
    \ingroup graphs_qml__2D
    \brief The legendData value type contains information to display on a sets
    legend marker.

    The information needed to make a visual association between a set and a
    marker include properties such as color, border color, and a name of a set.

    \note Before Qt 6.10, this value type was only provided as an anonymous
    type.
*/

/*!
    \property QLegendData::color
    \brief A legend marker's color.
*/
/*!
    \qmlproperty color legendData::color
    \brief A legend marker's color.
*/

/*!
    \property QLegendData::borderColor
    \brief A border color of a legend marker.
*/
/*!
    \qmlproperty color legendData::borderColor
    \brief A border color of a legend marker.
*/

/*!
    \property QLegendData::label
    \brief A name of a legend marker.
*/
/*!
    \qmlproperty string legendData::label
    \brief A name of a legend marker.
*/

/*!
    \enum QAbstractSeries::SeriesType

    This enum describes the type of the series.

    \value Line A line graph.
    \value Bar A bar graph.
    \value Scatter A scatter graph.
    \value Pie A pie graph.
    \value Spline A spline graph.
    \value Area An area graph.
    \value Custom A custom graph.
*/

/*!
    \property QAbstractSeries::type
    \brief The type of the series.
*/
/*!
    \qmlproperty enumeration AbstractSeries::type

    The type of the series.

    \value AbstractSeries.SeriesType.Line A line graph.
    \value AbstractSeries.SeriesType.Bar A bar graph.
    \value AbstractSeries.SeriesType.Scatter A scatter graph.
    \value AbstractSeries.SeriesType.Pie A pie graph.
    \value AbstractSeries.SeriesType.Spline A spline graph.
    \value AbstractSeries.SeriesType.Area An area graph.
    \value AbstractSeries.SeriesType.Custom A custom graph.
*/

/*!
    \property QAbstractSeries::name
    \brief The name of the series.

    The name is displayed in the legend for the series and it supports HTML formatting.
*/
/*!
    \qmlproperty string AbstractSeries::name
    The name of the series. The name is displayed in the legend for the series and it
    supports HTML formatting.
*/

/*!
    \property QAbstractSeries::visible
    \brief Visibility of the series.

    The visibility used for this series. By default, \a visible is set to \c true.
*/
/*!
    \qmlproperty bool AbstractSeries::visible
    The visibility used for this series. By default, \a visible is set to \c true.
*/

/*!
    \property QAbstractSeries::selectable
    \brief Controls if the series is selectable.

    Controls if the series can be selected with mouse/touch.
    By default, \a selectable is set to \c false.
*/
/*!
    \qmlproperty bool AbstractSeries::selectable
    Controls if the series can be selected with mouse/touch.
    By default, \a selectable is set to \c false.
*/

/*!
    \property QAbstractSeries::hoverable
    \brief Controls if the series is hoverable.

    Controls if the series can be hovered with mouse/touch.
    By default, \a hoverable is set to \c false.
*/
/*!
    \qmlproperty bool AbstractSeries::hoverable
    Controls if the series can be hovered with mouse/touch.
    By default, \a hoverable is set to \c false.
*/

/*!
    \property QAbstractSeries::hovered
    \brief Check whether a series is hovered on.

    Can be used to check whether mouse/touch is currently
    hovering on a series.
    \sa QAbstractSeries::hovered
*/
/*!
    \qmlproperty bool AbstractSeries::hovered
    Can be used to check whether mouse/touch is currently
    hovering on a series.
    \sa QAbstractSeries::hovered
*/

/*!
    \property QAbstractSeries::opacity
    \brief The opacity of the series.

    By default, the opacity is 1.0. The valid values range from 0.0 (transparent) to 1.0 (opaque).
*/
/*!
    \qmlproperty real AbstractSeries::opacity
    The opacity of the series. By default, the opacity is 1.0.
    The valid values range from 0.0 (transparent) to 1.0 (opaque).
*/

/*!
    \property QAbstractSeries::valuesMultiplier
    \brief Controls the series values effective visible value.

    This variable can be used for animating the series values so they scale from 0 to actual value size.
    By default, the valuesMultiplier is 1.0. The valid values range from 0.0 (height 0) to 1.0 (full value).
*/
/*!
    \qmlproperty real AbstractSeries::valuesMultiplier
    This variable can be used for animating the series values so they scale from 0 to actual value size.
    By default, the valuesMultiplier is 1.0. The valid values range from 0.0 (height 0) to 1.0 (full value).
*/

/*!
    \property QAbstractSeries::axisX
    \brief X-axis of this series.
    \since 6.10

    The x-axis used for this series. Creates a separate axis from the one defined
    in GraphsView showing the user multiple axis per graph.
*/
/*!
    \qmlproperty AbstractAxis AbstractSeries::axisX
    \since 6.10
    The x-axis used for this series. Creates a separate axis from the one defined
    in GraphsView showing the user multiple axis per graph.
    \sa axisY
*/

/*!
    \property QAbstractSeries::axisY
    \brief Y-axis of this series.
    \since 6.10

    The y-axis used for this series. Creates a separate axis from the one defined
    in GraphsView showing the user multiple axis per graph.
*/
/*!
    \qmlproperty AbstractAxis AbstractSeries::axisY
    \since 6.10
    The y-axis used for this series. Creates a separate axis from the one defined
    in GraphsView showing the user multiple axis per graph.
    \sa axisX
*/

/*!
    \property QAbstractSeries::zValue
    \brief Controls the order in which the series is drawn
    \since 6.10

    The series list of GraphsView is sorted by the zValue property. Since each series type is
    rendered at once, the order mostly works as an internal order of each series type. The highest
    zValue of each series type determines the order of rendering among series types. The default
    value is 0.
*/
/*!
    \qmlproperty int AbstractSeries::zValue
    \since 6.10
    The series list of GraphsView is sorted by the zValue property. Since each series type is
    rendered at once, the order mostly works as an internal order of each series type. The highest
    zValue of each series type determines the order of rendering among series types. The default
    value is 0.
*/

/*!
    \property QAbstractSeries::legendData
    \brief Contains information needed to create a legend marker for a data set in a graph.
    \sa QLegendData
 */
/*!
    \qmlproperty list<legendData> AbstractSeries::legendData
    Contains information needed to create a legend marker for a data set in a graph.
*/

/*!
    \qmlsignal AbstractSeries::legendDataChanged()
    This signal is emitted when legend data changes.
*/

/*!
    \qmlsignal AbstractSeries::themeChanged()
    This signal is emitted when the series theme changes.
*/

/*!
    \qmlsignal AbstractSeries::nameChanged()
    This signal is emitted when the series \l name changes.
*/

/*!
    \qmlsignal AbstractSeries::visibleChanged()
    This signal is emitted when the series visibility changes.
*/

/*!
    \qmlsignal AbstractSeries::selectableChanged()
    This signal is emitted when the series \l selectable changes.
*/

/*!
    \qmlsignal AbstractSeries::hoverableChanged()
    This signal is emitted when the series \l hoverable changes.
*/

/*!
    \qmlsignal AbstractSeries::hoveredChanged()
    This signal is emitted when the series \l hovered changes.
*/

/*!
    \qmlsignal AbstractSeries::opacityChanged()
    This signal is emitted when the \l opacity of the series changes.
*/

/*!
    \qmlsignal AbstractSeries::valuesMultiplierChanged()
    This signal is emitted when the valuesMultiplier of the series changes.
*/

/*!
    \qmlsignal AbstractSeries::axisXChanged(AbstractAxis newAxis)
    \since 6.10
    This signal is emitted whenever the X axis in control changes.
    The \a newAxis parameter holds the new axis.
*/

/*!
    \qmlsignal AbstractSeries::axisYChanged(AbstractAxis newAxis)
    \since 6.10
    This signal is emitted whenever the Y axis in control changes.
    The \a newAxis parameter holds the new axis.
*/

/*!
    \qmlsignal AbstractSeries::zValueChanged(int newDrawOrder)
    \since 6.10
    This signal is emitted when the series draw order changes to
    \a newDrawOrder.
*/

/*!
    \fn void QAbstractSeries::hoverEnter(const QString &seriesName, QPointF position, QPointF value)
    This signal is emitted when the series hovering starts. The name of the series is in \a seriesName,
    the mouse/touch position in \a position, and the series value in \a value.
    \note This signal is only emitted when \l hoverable is set to true.
    \note For Pie graph, the value represents (angle of position, start angle of hovering slice)
*/

/*!
    \fn void QAbstractSeries::hoverExit(const QString &seriesName, QPointF position)
    This signal is emitted when the series hovering ends. The name of the series is in \a seriesName,
    and the mouse/touch position in \a position.
    \note This signal is only emitted when \l hoverable is set to true.
*/

/*!
    \fn void QAbstractSeries::hover(const QString &seriesName, QPointF position, QPointF value)
    This signal is emitted when the series hovering changes. The name of the series is in \a seriesName,
    the mouse/touch position in \a position, and the series value in \a value.
    \note This signal is only emitted when \l hoverable is set to true.
    \note For Pie graph, the value represents (angle of position, start angle of hovering slice)
*/

/*!
    \internal
    \brief Constructs QAbstractSeries object with \a parent.
*/

Q_LOGGING_CATEGORY(lcSeries2D, "qt.graphs2d.series")
Q_LOGGING_CATEGORY(lcProperties2D, "qt.graphs2d.series.properties")

QAbstractSeries::QAbstractSeries(QAbstractSeriesPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{}

/*!
    \brief Virtual destructor for the graph series.
*/
QAbstractSeries::~QAbstractSeries()
{
    Q_D(QAbstractSeries);
    if (d->m_graph)
        d->m_graph->removeSeries(this);
}

QString QAbstractSeries::name() const
{
    Q_D(const QAbstractSeries);
    return d->m_name;
}

void QAbstractSeries::setName(const QString &name)
{
    Q_D(QAbstractSeries);
    if (name != d->m_name) {
        d->m_name = name;
        emit update();
        emit nameChanged();
    } else {
        qCDebug(lcProperties2D,"QAbstractSeries::setName. Name is already set to: %s",
                qPrintable(name));
    }
}

bool QAbstractSeries::isVisible() const
{
    Q_D(const QAbstractSeries);
    return d->m_visible;
}

void QAbstractSeries::setVisible(bool visible)
{
    Q_D(QAbstractSeries);
    if (visible != d->m_visible) {
        d->m_visible = visible;
        emit update();
        emit visibleChanged();
    } else {
        qCDebug(lcProperties2D) << "QAbstractSeries::setVisible. series visibility already set to"
                                << visible;
    }
}

bool QAbstractSeries::isSelectable() const
{
    Q_D(const QAbstractSeries);
    return d->m_selectable;
}

void QAbstractSeries::setSelectable(bool selectable)
{
    Q_D(QAbstractSeries);
    if (selectable != d->m_selectable) {
        d->m_selectable = selectable;
        emit update();
        emit selectableChanged();
    } else {
        qCDebug(lcProperties2D) << "QAbstractSeries::setSelectable. Selectable already set to:"
                                << selectable;
    }
}

bool QAbstractSeries::isHoverable() const
{
    Q_D(const QAbstractSeries);
    return d->m_hoverable;
}

void QAbstractSeries::setHoverable(bool hoverable)
{
    Q_D(QAbstractSeries);
    if (hoverable != d->m_hoverable) {
        d->m_hoverable = hoverable;
        emit update();
        emit hoverableChanged();
    } else {
        qCDebug(lcProperties2D) << "QAbstractSeries::setHoverable. Hoverable already set to:"
                                << hoverable;
    }
}

bool QAbstractSeries::hasLoaded() const
{
    Q_D(const QAbstractSeries);
    return d->m_loaded;
}

bool QAbstractSeries::isHovered() const
{
    Q_D(const QAbstractSeries);
    return d->m_hovered;
}

void QAbstractSeries::setHovered(bool enabled)
{
    Q_D(QAbstractSeries);

    if (enabled != d->m_hovered) {
        d->m_hovered = enabled;
        emit hoveredChanged(d->m_hovered);
    }
}

QAbstractAxis *QAbstractSeries::axisX() const
{
    Q_D(const QAbstractSeries);
    return d->m_axisX;
}

void QAbstractSeries::setAxisX(QAbstractAxis *newAxisX)
{
    Q_D(QAbstractSeries);
    if (d->m_axisX == newAxisX)
        return;

    if (d->m_axisX) {
        disconnect(d->m_axisX, &QAbstractAxis::update, this, &QAbstractSeries::update);

        if (d->m_graph)
            d->m_graph->removeAxis(d->m_axisX);
    }

    if (newAxisX) {
        if (newAxisX->alignment() != Qt::AlignBottom && newAxisX->alignment() != Qt::AlignTop)
            newAxisX->setAlignment(Qt::AlignBottom);
        connect(newAxisX, &QAbstractAxis::update, this, &QAbstractSeries::update);

        if (d->m_graph)
            d->m_graph->addAxis(newAxisX);
    }

    d->m_axisX = newAxisX;
    emit update();
    emit axisXChanged(newAxisX);
}

QAbstractAxis *QAbstractSeries::axisY() const
{
    Q_D(const QAbstractSeries);
    return d->m_axisY;
}

void QAbstractSeries::setAxisY(QAbstractAxis *newAxisY)
{
    Q_D(QAbstractSeries);
    if (d->m_axisY == newAxisY)
        return;

    if (d->m_axisY) {
        disconnect(d->m_axisY, &QAbstractAxis::update, this, &QAbstractSeries::update);

        if (d->m_graph)
            d->m_graph->removeAxis(d->m_axisY);
    }

    if (newAxisY) {
        if (newAxisY->alignment() != Qt::AlignLeft && newAxisY->alignment() != Qt::AlignRight)
            newAxisY->setAlignment(Qt::AlignLeft);
        connect(newAxisY, &QAbstractAxis::update, this, &QAbstractSeries::update);

        if (d->m_graph)
            d->m_graph->addAxis(newAxisY);
    }

    d->m_axisY = newAxisY;
    emit update();
    emit axisYChanged(newAxisY);
}

qreal QAbstractSeries::opacity() const
{
    Q_D(const QAbstractSeries);
    return d->m_opacity;
}

void QAbstractSeries::setOpacity(qreal opacity)
{
    Q_D(QAbstractSeries);
    if (opacity != d->m_opacity) {
        d->m_opacity = opacity;
        emit update();
        emit opacityChanged();
    } else {
        qCDebug(lcProperties2D, "QAbstractSeries::setOpacity. Opacity is already set to: %f",
                opacity);
    }
}

qreal QAbstractSeries::valuesMultiplier() const
{
    Q_D(const QAbstractSeries);
    return d->m_valuesMultiplier;
}

void QAbstractSeries::setValuesMultiplier(qreal valuesMultiplier)
{
    Q_D(QAbstractSeries);
    valuesMultiplier = std::clamp<qreal>(valuesMultiplier, 0.0, 1.0);
    if (valuesMultiplier != d->m_valuesMultiplier) {
        d->m_valuesMultiplier = valuesMultiplier;
        emit update();
        emit valuesMultiplierChanged();
    }
}

int QAbstractSeries::zValue() const
{
    Q_D(const QAbstractSeries);
    return d->m_drawOrder;
}

void QAbstractSeries::setZValue(int newDrawOrder)
{
    Q_D(QAbstractSeries);
    if (d->m_drawOrder == newDrawOrder)
        return;
    d->m_drawOrder = newDrawOrder;
    emit update();
    emit zValueChanged(newDrawOrder);
}

/*!
    \internal
    Returns the graph that the series belongs to.

    Set automatically when the series is added to the graph,
    and unset when the series is removed from the graph.
*/
QGraphsView *QAbstractSeries::graph() const
{
    Q_D(const QAbstractSeries);
    return d->m_graph;
}

void QAbstractSeries::setGraph(QGraphsView *graph)
{
    Q_D(QAbstractSeries);
    if (graph && d->m_graph != graph) {
        if (d->m_graph) {
            if (d->m_axisX)
                d->m_graph->removeAxis(d->m_axisX);
            if (d->m_axisY)
                d->m_graph->removeAxis(d->m_axisY);
        }
        if (d->m_axisX)
            graph->addAxis(d->m_axisX);
        if (d->m_axisY)
            graph->addAxis(d->m_axisY);
    } else if (!graph && d->m_graph) {
        if (d->m_axisX)
            d->m_graph->removeAxis(d->m_axisX);
        if (d->m_axisY)
            d->m_graph->removeAxis(d->m_axisY);
    }
    d->m_graph = graph;
    if (graph) {
        switch (type()) {
        case SeriesType::Bar:
#ifdef USE_BARGRAPH
            graph->createBarsRenderer();
#endif
            break;
        case SeriesType::Scatter:
        case SeriesType::Line:
        case SeriesType::Spline:
#ifdef USE_POINTS
            graph->createPointRenderer();
#endif
            break;
        case SeriesType::Pie:
#ifdef USE_PIEGRAPH
            graph->createPieRenderer();
#endif
            break;
        case SeriesType::Area:
#ifdef USE_AREAGRAPH
            graph->createAreaRenderer();
#endif
            break;
        case SeriesType::Custom:
#ifdef USE_CUSTOMGRAPH
            graph->createCustomRenderer();
#endif
            break;
        default:
            break;
        }
    }
}

/*!
    Sets the visibility of the series to \c true.

    \sa setVisible(), isVisible()
*/
void QAbstractSeries::show()
{
    setVisible(true);
}

/*!
    Sets the visibility of the series to \c false.

    \sa setVisible(), isVisible()
*/
void QAbstractSeries::hide()
{
    setVisible(false);
}

const QList<QLegendData> QAbstractSeries::legendData() const
{
    Q_D(const QAbstractSeries);
    return d->m_legendData;
}

/*!
    \property QAbstractSeries::seriesChildren

    The list of series child objects. This is a default property that allows child objects to be
    specified within a series element in QML without explicitly using the children property name.
*/
QQmlListProperty<QObject> QAbstractSeries::seriesChildren()
{
    return QQmlListProperty<QObject>(this, 0, &QAbstractSeriesPrivate::appendSeriesChildren, 0, 0, 0);
}

void QAbstractSeries::classBegin()
{
}

void QAbstractSeries::componentComplete()
{
    Q_D(QAbstractSeries);
    d->m_loaded = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QAbstractSeriesPrivate::QAbstractSeriesPrivate(QAbstractSeries::SeriesType type)
    : m_type(type)
{
}

QAbstractSeriesPrivate::~QAbstractSeriesPrivate()
{
}

void QAbstractSeriesPrivate::removeAxis(QAbstractAxis *axis)
{
    if (m_axisX == axis)
        m_axisX = nullptr;
    if (m_axisY == axis)
        m_axisY = nullptr;
}

void QAbstractSeriesPrivate::setLegendData(const QList<QLegendData> &legendData)
{
    if (legendData.data() != m_legendData.data()) {
        Q_Q(QAbstractSeries);
        m_legendData = legendData;
        emit q->legendDataChanged();
    }
}

void QAbstractSeriesPrivate::clearLegendData()
{
    if (!m_legendData.empty()) {
        Q_Q(QAbstractSeries);
        m_legendData.clear();
        emit q->legendDataChanged();
    }
}

void QAbstractSeriesPrivate::appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element)
{
    // Empty implementation; the children are parsed in componentComplete instead
    Q_UNUSED(list);
    Q_UNUSED(element);
}
QT_END_NAMESPACE

#include "moc_qabstractseries.cpp"
