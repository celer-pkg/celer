// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/qabstractseries.h"
#include "graphs2d/qabstractseries_p.h"
#ifdef USE_AREAGRAPH
#include <QtGraphs/qareaseries.h>
#include <private/arearenderer_p.h>
#endif
#ifdef USE_BARGRAPH
#include <QtGraphs/qbarseries.h>
#include <private/barsrenderer_p.h>
#endif
#ifdef USE_PIEGRAPH
#include <QtGraphs/qpieseries.h>
#include <private/pierenderer_p.h>
#endif
#ifdef USE_LINEGRAPH
#include <QtGraphs/qlineseries.h>
#endif
#ifdef USE_SCATTERGRAPH
#include <QtGraphs/qscatterseries.h>
#endif
#ifdef USE_SPLINEGRAPH
#include <QtGraphs/qsplineseries.h>
#endif
#ifdef USE_POINTS
#include <private/pointrenderer_p.h>
#endif
#ifdef USE_CUSTOMGRAPH
#include <QtGraphs/qcustomseries.h>
#include <private/customrenderer_p.h>
#endif
#include <QTimer>
#include <QtQuick/private/qquickpinchhandler_p.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <QtCore/qset.h>
#include <private/axisrenderer_p.h>
#include <private/qabstractaxis_p.h>
#include <private/qgraphsview_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
               "class QGraphsView;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsVieUpdatePolish_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsVieUpdatePolish_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewComponentComplete_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewComponentComplete_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewInsertSeries_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewInsertSeries_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateBarsRenderer_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateBarsRenderer_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateAxisRenderer_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateAxisRenderer_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreatePointRenderer_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreatePointRenderer_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreatePieRenderer_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreatePieRenderer_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateAreaRenderer_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DGraphsViewCreateAreaRenderer_exit);

/*!
    \class QGraphsView
    \inmodule QtGraphs
    \internal
*/

/*!
    \qmltype GraphsView
    \nativetype QGraphsView
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \brief Base type for all Qt Graphs views.

This class collects the series and theming together and draws the graphs.
You will need to import Qt Graphs module to use this type:

\snippet doc_src_qmlgraphs.cpp 0

After that you can use GraphsView in your qml files:

\snippet doc_src_qmlgraphs.cpp 10

\image graphsview-minimal.png

\sa BarSeries, LineSeries, BarCategoryAxis, ValueAxis, GraphsTheme
*/

Q_LOGGING_CATEGORY(lcGraphs2D, "qt.graphs2d.general")
Q_LOGGING_CATEGORY(lcViewProperties2D, "qt.graphs2d.graphsview.properties")
Q_LOGGING_CATEGORY(lcEvents2D, "qt.graphs2d.events")
Q_LOGGING_CATEGORY(lcCritical2D, "qt.graphs2d.critical")

QGraphsView::QGraphsView(QQuickItem *parent) :
    QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
    m_defaultTheme = new QGraphsTheme(this);
    m_pinchHandler = new QQuickPinchHandler(this);
    m_pinchHandler->setTarget(nullptr);

    QObject::connect(m_pinchHandler,
                     &QQuickPinchHandler::scaleChanged,
                     this,
                     &QGraphsView::onPinchScaleChanged);
    QObject::connect(m_pinchHandler,
                     &QQuickPinchHandler::grabChanged,
                     this,
                     &QGraphsView::onPinchGrabChanged);
}

QGraphsView::~QGraphsView()
{
    const auto slist = m_seriesList;
    for (const auto &s : slist)
        removeSeries(s);
    if (m_axisX)
        m_axisX->d_func()->setGraph(nullptr);
    if (m_axisY)
        m_axisY->d_func()->setGraph(nullptr);
}

void QGraphsView::onPinchScaleChanged(qreal delta)
{
    if (m_axisRenderer)
        m_axisRenderer->handlePinchScale(delta);
}

void QGraphsView::onPinchGrabChanged(QPointingDevice::GrabTransition transition, QEventPoint point)
{
    if (m_axisRenderer)
        m_axisRenderer->handlePinchGrab(transition, point);
}

/*!
    \qmlmethod void GraphsView::addSeries(AbstractSeries series)
    Appends a \a series into GraphsView.
    If the \a series is null, it will not be added. If the \a series already
    belongs to the graph, it will be moved into the end.
*/
/*!
    Appends a \a series into GraphsView.
    If the \a series is null, it will not be added. If the \a series already
    belongs to the graph, it will be moved into the end.
*/
void QGraphsView::addSeries(QObject *series)
{
    insertSeries(m_seriesList.size(), series);
}

/*!
    \qmlmethod void GraphsView::insertSeries(int index, AbstractSeries series)
    Inserts a \a series at the position specified by \a index.
    If the \a series is null, it will not be inserted. If the \a series already
    belongs to the graph, it will be moved into \a index.
*/
/*!
    Inserts a \a series at the position specified by \a index.
    If the \a series is null, it will not be inserted. If the \a series already
    belongs to the graph, it will be moved into \a index.
*/
void QGraphsView::insertSeries(qsizetype index, QObject *object)
{
    if (auto series = qobject_cast<QAbstractSeries *>(object)) {
        Q_TRACE(QGraphs2DGraphsViewInsertSeries_entry);
        series->setGraph(this);
        if (m_seriesList.contains(series)) {
            qsizetype oldIndex = m_seriesList.indexOf(series);
            if (index != oldIndex) {
                m_seriesList.removeOne(series);
                if (oldIndex < index)
                    index--;
                m_seriesList.insert(index, series);
                qCDebug(lcGraphs2D, "series was already in seriesList, removed old series at index: %" PRIdQSIZETYPE
                        " and inserted new one at index: %" PRIdQSIZETYPE,
                        oldIndex, index);
            }
        } else {
            m_seriesList.insert(index, series);

            QObject::connect(series,
                             &QAbstractSeries::update,
                             this,
                             &QGraphsView::updateComponentSizes);
            QObject::connect(series, &QAbstractSeries::update,
                             this, &QGraphsView::polishAndUpdate);
            QObject::connect(series, &QAbstractSeries::hoverEnter,
                             this, &QGraphsView::handleHoverEnter);
            QObject::connect(series, &QAbstractSeries::hoverExit,
                             this, &QGraphsView::handleHoverExit);
            QObject::connect(series, &QAbstractSeries::hover,
                             this, &QGraphsView::handleHover);

#ifdef USE_PIEGRAPH
            if (auto pie = qobject_cast<QPieSeries *>(series))
                connect(pie, &QPieSeries::removed, m_pieRenderer, &PieRenderer::markedDeleted);
#endif
            qCDebug(lcGraphs2D) << series << "added to a list at index of" << index;
        }
        Q_TRACE(QGraphs2DGraphsViewInsertSeries_exit);

        updateComponentSizes();
        polishAndUpdate();
    }
}

/*!
    \qmlmethod void GraphsView::removeSeries(AbstractSeries series)
    Removes the \a series from the graph.
*/
/*!
    Removes the \a series from the graph.
*/
void QGraphsView::removeSeries(QObject *object)
{
    if (auto series = qobject_cast<QAbstractSeries *>(object)) {
        series->setGraph(nullptr);
        m_seriesList.removeAll(series);
        auto &cleanupSeriesList = m_cleanupSeriesList[getSeriesRendererIndex(series)];

#ifdef USE_PIEGRAPH
        if (auto pie = qobject_cast<QPieSeries *>(series))
            disconnect(pie, &QPieSeries::removed, m_pieRenderer, &PieRenderer::markedDeleted);
#endif
        qCDebug(lcGraphs2D) << "removing" << series << "from seriesList";
        cleanupSeriesList.append(series);
        updateComponentSizes();
        polishAndUpdate();
    }
}

/*!
    \qmlmethod void GraphsView::removeSeries(int index)
    Removes the series specified by \a index from the graph.
*/
/*!
    Removes the series specified by \a index from the graph.
*/
void QGraphsView::removeSeries(qsizetype index)
{
    if (index >= 0 && index < m_seriesList.size())
        removeSeries(m_seriesList[index]);
}

/*!
    \qmlmethod bool GraphsView::hasSeries(AbstractSeries series)
    Returns \c true if the \a series is in the graph.
*/
/*!
    Returns \c true if the \a series is in the graph.
*/
bool QGraphsView::hasSeries(QObject *series)
{
    return m_seriesList.contains(series);
}

QPointF QGraphsView::getDataPointCoordinates(QAbstractSeries *series, qreal x, qreal y)
{
#ifdef USE_LINEGRAPH
    if (m_pointRenderer)
        return m_pointRenderer->reverseRenderCoordinates(series, x, y);
#else
    Q_UNUSED(series);
    Q_UNUSED(x);
    Q_UNUSED(y);
#endif
    return QPointF();
}


void QGraphsView::addAxis(QAbstractAxis *axis)
{
    if (axis) {
        axis->d_func()->setGraph(this);
        // Ensure AxisRenderer exists
        createAxisRenderer();
        polishAndUpdate();
        QObject::connect(axis, &QAbstractAxis::update, this, &QGraphsView::polishAndUpdate);
        QObject::connect(axis, &QAbstractAxis::update, this, &QGraphsView::updateComponentSizes);
    }
}

void QGraphsView::removeAxis(QAbstractAxis *axis, bool removeAllReferences)
{
    if (axis) {
        axis->d_func()->setGraph(nullptr);
        QObject::disconnect(axis, &QAbstractAxis::update, this, &QGraphsView::polishAndUpdate);
        QObject::disconnect(axis,
                            &QAbstractAxis::update,
                            this,
                             &QGraphsView::updateComponentSizes);
    }

    if (m_axisX == axis)
        m_axisX = nullptr;
    if (m_axisY == axis)
        m_axisY = nullptr;

    if (removeAllReferences) {
        for (auto&& s : m_seriesList) {
            if (auto series = qobject_cast<QAbstractSeries *>(s))
                series->d_func()->removeAxis(axis);
        }
    }

    updateComponentSizes();
    polishAndUpdate();
}

qsizetype QGraphsView::graphSeriesCount() const
{
    return m_graphSeriesCount;
}

void QGraphsView::setGraphSeriesCount(qsizetype count)
{
    if (count > m_graphSeriesCount)
        m_graphSeriesCount = count;
}

#ifdef USE_BARGRAPH
void QGraphsView::createBarsRenderer()
{
    Q_TRACE_SCOPE(QGraphs2DGraphsViewCreateBarsRenderer);
    if (!m_barsRenderer) {
        qCDebug(lcGraphs2D, "creating bars renderer");
        m_barsRenderer = new BarsRenderer(this, clipPlotArea());
        updateComponentSizes();
    }
}
#endif

void QGraphsView::createAxisRenderer()
{
    Q_TRACE_SCOPE(QGraphs2DGraphsViewCreateAxisRenderer);
    if (!m_axisRenderer) {
        qCDebug(lcGraphs2D) << "creating axis renderer.";
        m_axisRenderer = new AxisRenderer(this);
        m_axisRenderer->setZ(-1);
        updateComponentSizes();
    }
}

#ifdef USE_POINTS
void QGraphsView::createPointRenderer()
{
    Q_TRACE_SCOPE(QGraphs2DGraphsViewCreatePointRenderer);
    if (!m_pointRenderer) {
        qCDebug(lcGraphs2D, "creating point renderer.");
        m_pointRenderer = new PointRenderer(this, clipPlotArea());
        updateComponentSizes();
    }
}
#endif

#ifdef USE_PIEGRAPH
void QGraphsView::createPieRenderer()
{
    Q_TRACE_SCOPE(QGraphs2DGraphsViewCreatePieRenderer);
    if (!m_pieRenderer) {
        qCDebug(lcGraphs2D, "creating pie renderer.");
        m_pieRenderer = new PieRenderer(this, clipPlotArea());
        updateComponentSizes();
    }
}
#endif

#ifdef USE_AREAGRAPH
void QGraphsView::createAreaRenderer()
{
    Q_TRACE_SCOPE(QGraphs2DGraphsViewCreateAreaRenderer);
    if (!m_areaRenderer) {
        qCDebug(lcGraphs2D, "creating area renderer.");
        m_areaRenderer = new AreaRenderer(this, clipPlotArea());
        updateComponentSizes();
    }
}
#endif

#ifdef USE_CUSTOMGRAPH
void QGraphsView::createCustomRenderer()
{
    if (!m_customRenderer) {
        qCDebug(lcGraphs2D, "creating custom renderer.");
        m_customRenderer = new CustomRenderer(this, clipPlotArea());
        updateComponentSizes();
    }
}
#endif

/*!
    \property QGraphsView::axisXSmoothing
    \brief Controls the graph X axis smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
/*!
    \qmlproperty real GraphsView::axisXSmoothing
    Controls the graph X axis smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
qreal QGraphsView::axisXSmoothing() const
{
    return m_axisXSmoothing;
}

void QGraphsView::setAxisXSmoothing(qreal smoothing)
{
    if (QtPrivate::fuzzyCompare(m_axisXSmoothing, smoothing)) {
        qCDebug(lcViewProperties2D, "%s axis smoothing is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)),
                smoothing);
        return;
    }
    m_axisXSmoothing = smoothing;
    emit axisXSmoothingChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::axisYSmoothing
    \brief Controls the graph Y axis smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
/*!
    \qmlproperty real GraphsView::axisYSmoothing
    Controls the graph Y axis smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
qreal QGraphsView::axisYSmoothing() const
{
    return m_axisYSmoothing;
}

void QGraphsView::setAxisYSmoothing(qreal smoothing)
{
    if (QtPrivate::fuzzyCompare(m_axisYSmoothing, smoothing)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), smoothing);
        return;
    }
    m_axisYSmoothing = smoothing;
    emit axisYSmoothingChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::gridSmoothing
    \brief Controls the graph grid smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
/*!
    \qmlproperty real GraphsView::gridSmoothing
    Controls the graph grid smoothing (antialiasing) amount.
    By default, the smoothing is \c 1.0.
*/
qreal QGraphsView::gridSmoothing() const
{
    return m_gridSmoothing;
}

void QGraphsView::setGridSmoothing(qreal smoothing)
{
    if (QtPrivate::fuzzyCompare(m_gridSmoothing, smoothing)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), smoothing);
        return;
    }
    m_gridSmoothing = smoothing;
    emit gridSmoothingChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowVisible
    \brief Controls if the graph grid shadow is visible.
    By default, shadow visibility is set to \c false.
*/
/*!
    \qmlproperty bool GraphsView::shadowVisible
    Controls if the graph grid shadow is visible.
    By default, shadow visibility is set to \c false.
*/
bool QGraphsView::isShadowVisible() const
{
    return m_isShadowVisible;
}

void QGraphsView::setShadowVisible(bool newShadowVisibility)
{
    if (m_isShadowVisible == newShadowVisibility) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newShadowVisibility;
        return;
    }
    m_isShadowVisible = newShadowVisibility;
    emit shadowVisibleChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowColor
    \brief Controls the graph grid shadow color.
    By default, shadow color is set to \c black.
*/
/*!
    \qmlproperty color GraphsView::shadowColor
    Controls the graph grid shadow color.
    By default, shadow color is set to \c black.
*/
QColor QGraphsView::shadowColor() const
{
    return m_shadowColor;
}

void QGraphsView::setShadowColor(QColor newShadowColor)
{
    if (m_shadowColor == newShadowColor) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(newShadowColor.name()));
        return;
    }
    m_shadowColor = newShadowColor;
    emit shadowColorChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowBarWidth
    \brief Controls the graph grid shadow width.
    By default, shadow width is set to \c 2.0.
*/
/*!
    \qmlproperty real GraphsView::shadowBarWidth
    Controls the graph grid shadow width.
    By default, shadow width is set to \c 2.0.
*/
qreal QGraphsView::shadowBarWidth() const
{
    return m_shadowBarWidth;
}

void QGraphsView::setShadowBarWidth(qreal newShadowBarWidth)
{
    if (QtPrivate::fuzzyCompare(m_shadowBarWidth, newShadowBarWidth)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newShadowBarWidth);
        return;
    }
    m_shadowBarWidth = newShadowBarWidth;
    emit shadowBarWidthChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowXOffset
    \brief Controls the graph grid shadow X offset.
    By default, shadow X offset is set to \c 0.0.
*/
/*!
    \qmlproperty real GraphsView::shadowXOffset
    Controls the graph grid shadow X offset.
    By default, shadow X offset is set to \c 0.0.
*/
qreal QGraphsView::shadowXOffset() const
{
    return m_shadowXOffset;
}

void QGraphsView::setShadowXOffset(qreal newShadowXOffset)
{
    if (QtPrivate::fuzzyCompare(m_shadowXOffset, newShadowXOffset)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newShadowXOffset);
        return;
    }
    m_shadowXOffset = newShadowXOffset;
    emit shadowXOffsetChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowYOffset
    \brief Controls the graph grid shadow Y offset.
    By default, shadow Y offset is set to \c 0.0.
*/
/*!
    \qmlproperty real GraphsView::shadowYOffset
    Controls the graph grid shadow Y offset.
    By default, shadow Y offset is set to \c 0.0.
*/
qreal QGraphsView::shadowYOffset() const
{
    return m_shadowYOffset;
}

void QGraphsView::setShadowYOffset(qreal newShadowYOffset)
{
    if (QtPrivate::fuzzyCompare(m_shadowYOffset, newShadowYOffset)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newShadowYOffset);
        return;
    }
    m_shadowYOffset = newShadowYOffset;
    emit shadowYOffsetChanged();
    polishAndUpdate();
}

/*!
    \property QGraphsView::shadowSmoothing
    \brief Controls the graph grid shadow smoothing (antialiasing) amount.
    By default, shadow smoothing is set to \c 4.0.
*/
/*!
    \qmlproperty real GraphsView::shadowSmoothing
    Controls the graph grid shadow smoothing (antialiasing) amount.
    By default, shadow smoothing is set to \c 4.0.
*/
qreal QGraphsView::shadowSmoothing() const
{
    return m_shadowSmoothing;
}

void QGraphsView::setShadowSmoothing(qreal smoothing)
{
    if (QtPrivate::fuzzyCompare(m_shadowSmoothing, smoothing)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), smoothing);
        return;
    }
    m_shadowSmoothing = smoothing;
    emit shadowSmoothingChanged();
    polishAndUpdate();
}

void QGraphsView::handleHoverEnter(const QString &seriesName, QPointF position, QPointF value)
{
    if (m_hoverCount == 0)
        emit hoverEnter(seriesName, position, value);
    m_hoverCount++;
}

void QGraphsView::handleHoverExit(const QString &seriesName, QPointF position)
{
    m_hoverCount--;
    if (m_hoverCount == 0)
        emit hoverExit(seriesName, position);
}

void QGraphsView::handleHover(const QString &seriesName, QPointF position, QPointF value)
{
    emit hover(seriesName, position, value);
}

void QGraphsView::updateComponentSizes()
{
    qCDebug(lcEvents2D, "updating component sizes.");
    updateAxisAreas();
    updatePlotArea();

    if (m_axisRenderer)
        m_axisRenderer->setSize(size());

#ifdef USE_BARGRAPH
    if (m_barsRenderer) {
        m_barsRenderer->setX(m_plotArea.x());
        m_barsRenderer->setY(m_plotArea.y());
        m_barsRenderer->setSize(m_plotArea.size());
        qCDebug(lcEvents2D) << "bars graph size:" << m_plotArea.size();
        qCDebug(lcEvents2D, "barsRenderer plotArea x: %f y: %f",
                m_plotArea.x(),
                m_plotArea.y());
    }
#endif
#ifdef USE_POINTS
    if (m_pointRenderer) {
        m_pointRenderer->setX(m_plotArea.x());
        m_pointRenderer->setY(m_plotArea.y());
        m_pointRenderer->setSize(m_plotArea.size());
        qCDebug(lcEvents2D) << "point graph size:" << m_plotArea.size();
        qCDebug(lcEvents2D, "pointRenderer plotArea x: %f y: %f",
                m_plotArea.x(),
                m_plotArea.y());

    }
#endif
#ifdef USE_PIEGRAPH
    if (m_pieRenderer) {
        m_pieRenderer->setX(m_plotArea.x());
        m_pieRenderer->setY(m_plotArea.y());
        m_pieRenderer->setSize(m_plotArea.size());
        qCDebug(lcEvents2D) << "pie graph size:" << m_plotArea.size();
        qCDebug(lcEvents2D, "pieRenderer plotArea x: %f y: %f",
                m_plotArea.x(),
                m_plotArea.y());

    }
#endif
#ifdef USE_AREAGRAPH
    if (m_areaRenderer) {
        m_areaRenderer->setX(m_plotArea.x());
        m_areaRenderer->setY(m_plotArea.y());
        m_areaRenderer->setSize(m_plotArea.size());
        qCDebug(lcEvents2D) << "area graph size:" << m_plotArea.size();
        qCDebug(lcEvents2D, "areaRenderer plotArea x: %f y: %f",
                m_plotArea.x(),
                m_plotArea.y());

    }
#endif
#ifdef USE_CUSTOMGRAPH
    if (m_customRenderer) {
        m_customRenderer->setX(m_plotArea.x());
        m_customRenderer->setY(m_plotArea.y());
        m_customRenderer->setSize(m_plotArea.size());
        qCDebug(lcEvents2D) << "custom graph size:" << m_plotArea.size();
        qCDebug(lcEvents2D, "customaRenderer plotArea x: %f y: %f", m_plotArea.x(), m_plotArea.y());
    }
#endif
}

void QGraphsView::componentComplete()
{
    Q_TRACE(QGraphs2DGraphsViewComponentComplete_entry);
    if (!m_zoomAreaDelegate && !m_zoomAreaItem) {
        const QString qmlData = QLatin1StringView(R"QML(
            import QtQuick;
            Rectangle {
                color: "#8888aaff"
                border.width: 1
                border.color: "#4466aa"
            }
        )QML");

        QQmlComponent *tempZoomAreaDelegate = new QQmlComponent(qmlEngine(this), this);
        tempZoomAreaDelegate->setData(qmlData.toUtf8(), QUrl());

        m_zoomAreaItem = qobject_cast<QQuickItem *>(
            tempZoomAreaDelegate->create(tempZoomAreaDelegate->creationContext()));
        m_zoomAreaItem->setParent(this);
        m_zoomAreaItem->setParentItem(this);
        m_zoomAreaItem->setVisible(false);
    }

    if (!m_theme) {
        m_theme = m_defaultTheme;
        QObject::connect(m_theme, &QGraphsTheme::update, this, &QQuickItem::update);
        m_theme->resetColorTheme();
    }
    Q_TRACE(QGraphs2DGraphsViewComponentComplete_exit);

    QQuickItem::componentComplete();

    qCDebug(lcEvents2D, "QGraphsView::componentComplete.");

    ensurePolished();
}

void QGraphsView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    qCDebug(lcEvents2D) << "QGraphsView::geometryChange."
                        << "oldGeometry:" << oldGeometry
                        << "newGeometry:" << newGeometry;

    QQuickItem::geometryChange(newGeometry, oldGeometry);

    updateComponentSizes();

    ensurePolished();
}

void QGraphsView::hoverMoveEvent(QHoverEvent *event)
{
    bool handled = false;

    // Adjust event position to renderers position
    QPointF localPos = event->position() - m_plotArea.topLeft();
    QHoverEvent mappedEvent(event->type(), localPos,event->globalPosition(),
                            event->oldPosF(), event->modifiers());
    mappedEvent.setAccepted(false);

#ifdef USE_BARGRAPH
    if (m_barsRenderer)
        handled |= m_barsRenderer->handleHoverMove(&mappedEvent);
#endif

#ifdef USE_POINTS
    if (m_pointRenderer)
        handled |= m_pointRenderer->handleHoverMove(&mappedEvent);
#endif

#ifdef USE_PIEGRAPH
    if (m_pieRenderer)
        handled |= m_pieRenderer->handleHoverMove(&mappedEvent);
#endif

#ifdef USE_AREAGRAPH
    if (m_areaRenderer)
        handled |= m_areaRenderer->handleHoverMove(&mappedEvent);
#endif

    if (!handled)
        event->ignore();
}

void QGraphsView::wheelEvent(QWheelEvent *event)
{
    bool handled = false;

    // Adjust event position to renderers position
    QPointF localPos = event->position() - m_plotArea.topLeft();
    QWheelEvent mappedEvent(localPos,
                            event->globalPosition(),
                            event->pixelDelta(),
                            event->angleDelta(),
                            event->buttons(),
                            event->modifiers(),
                            event->phase(),
                            event->inverted(),
                            event->source());
    mappedEvent.setAccepted(false);

    if (m_axisRenderer)
        handled |= m_axisRenderer->handleWheel(&mappedEvent);

    if (!handled)
        event->ignore();
    else
        polishAndUpdate();
}

QSGNode *QGraphsView::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData);

    for (auto series : std::as_const(m_seriesList)) {
        qCDebug(lcEvents2D) << "QGraphsView::updatePaintNode." << series;
#ifdef USE_BARGRAPH
        if (m_barsRenderer) {
            if (auto barSeries = qobject_cast<QBarSeries *>(series))
                m_barsRenderer->updateSeries(barSeries);
        }
#endif

#ifdef USE_POINTS
        if (m_pointRenderer) {
#ifdef USE_LINEGRAPH
            if (auto lineSeries = qobject_cast<QLineSeries *>(series))
                m_pointRenderer->updateSeries(lineSeries);
#endif
#ifdef USE_SCATTERGRAPH
            if (auto scatterSeries = qobject_cast<QScatterSeries *>(series))
                m_pointRenderer->updateSeries(scatterSeries);
#endif
#ifdef USE_SPLINEGRAPH
            if (auto splineSeries = qobject_cast<QSplineSeries *>(series))
                m_pointRenderer->updateSeries(splineSeries);
#endif
        }
#endif

#ifdef USE_PIEGRAPH
        if (m_pieRenderer) {
            if (auto pieSeries = qobject_cast<QPieSeries *>(series))
                m_pieRenderer->updateSeries(pieSeries);
        }
#endif

#ifdef USE_AREAGRAPH
        if (m_areaRenderer) {
            if (auto areaSeries = qobject_cast<QAreaSeries *>(series))
                m_areaRenderer->updateSeries(areaSeries);
        }
#endif

#ifdef USE_CUSTOMGRAPH
        if (m_customRenderer) {
            if (auto customSeries = qobject_cast<QCustomSeries *>(series))
                m_customRenderer->updateSeries(customSeries);
        }
#endif
    }

#ifdef USE_BARGRAPH
    if (m_barsRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[0];
        m_barsRenderer->afterUpdate(cleanupSeriesList);
        cleanupSeriesList.clear();
    }
#endif

#ifdef USE_POINTS
    if (m_pointRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[1];
        m_pointRenderer->afterUpdate(cleanupSeriesList);
        cleanupSeriesList.clear();
    }
#endif

#ifdef USE_AREAGRAPH
    if (m_areaRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[2];
        m_areaRenderer->afterUpdate(cleanupSeriesList);
    }
#endif

#ifdef USE_PIEGRAPH
    if (m_pieRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[3];
        m_pieRenderer->afterUpdate(cleanupSeriesList);
        cleanupSeriesList.clear();
    }
#endif

#ifdef USE_CUSTOMGRAPH
    if (m_customRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[4];
        m_customRenderer->afterUpdate(cleanupSeriesList);
        cleanupSeriesList.clear();
    }
#endif

    // Now possibly dirty theme has been taken into use
    m_theme->resetThemeDirty();

    return oldNode;
}

void QGraphsView::updatePolish()
{
    qCDebug(lcEvents2D, "QGraphsView::updatePolish. Start Update and polish.");

    Q_TRACE_SCOPE(QGraphs2DGraphsVieUpdatePolish);
    if (m_axisRenderer) {
        m_axisRenderer->handlePolish();
        // Initialize shaders after system's event queue
        QTimer::singleShot(0, m_axisRenderer, &AxisRenderer::initialize);
    }
    if (m_theme && m_theme->isBackgroundVisible()) {
        if (!m_backgroundRectangle) {
            // Create m_backgroundRectangle only when it is needed
            m_backgroundRectangle = new QQuickRectangle(this);
            m_backgroundRectangle->setZ(-2);
        }
        m_backgroundRectangle->setColor(m_theme->backgroundColor());
        m_backgroundRectangle->setWidth(width());
        m_backgroundRectangle->setHeight(height());
        m_backgroundRectangle->setVisible(true);
    } else if (m_backgroundRectangle) {
        // Hide and delete the m_backgroundRectangle
        m_backgroundRectangle->setVisible(false);
        m_backgroundRectangle->deleteLater();
        m_backgroundRectangle = nullptr;
    }

    std::sort(m_seriesList.begin(), m_seriesList.end(), [](QObject *lhs, QObject *rhs) {
        auto series1 = qobject_cast<QAbstractSeries *>(lhs);
        auto series2 = qobject_cast<QAbstractSeries *>(rhs);

        if (series1 && series2)
            return series1->zValue() < series2->zValue();
        return false;
    });

    #ifdef USE_POINTS
    if (m_pointRenderer)
        m_pointRenderer->resetShapePathCount();
    #endif

    #ifdef USE_AREAGRAPH
    if (m_areaRenderer)
        m_areaRenderer->resetShapePathCount();
    #endif

    float highestBarsZ = -std::numeric_limits<float>::max();
    float highestPointZ = -std::numeric_limits<float>::max();
    float highestPieZ = -std::numeric_limits<float>::max();
    float highestAreaZ = -std::numeric_limits<float>::max();
    float highestCustomZ = -std::numeric_limits<float>::max();

    // Polish for all series
#ifdef USE_BARGRAPH
    int barSeriesIndex = 0;
    int barSeriesCount =
            std::count_if(m_seriesList.begin(), m_seriesList.end(),
                          [](const auto &series) { return qobject_cast<QBarSeries *>(series); });
#endif
    for (auto series : std::as_const(m_seriesList)) {
#ifdef USE_BARGRAPH
        if (m_barsRenderer) {
            if (auto barSeries = qobject_cast<QBarSeries *>(series)) {
                m_barsRenderer->handlePolish(barSeries, barSeriesIndex, barSeriesCount);
                if (barSeries->zValue() > highestBarsZ)
                    highestBarsZ = barSeries->zValue();
                barSeriesIndex++;
            }
        }
#endif

#ifdef USE_POINTS
        if (m_pointRenderer) {
#ifdef USE_LINEGRAPH
            if (auto lineSeries = qobject_cast<QLineSeries *>(series)) {
                m_pointRenderer->handlePolish(lineSeries);
                if (lineSeries->zValue() > highestPointZ)
                    highestPointZ = lineSeries->zValue();
            }
#endif

#ifdef USE_SCATTERGRAPH
            if (auto scatterSeries = qobject_cast<QScatterSeries *>(series)) {
                m_pointRenderer->handlePolish(scatterSeries);
                if (scatterSeries->zValue() > highestPointZ)
                    highestPointZ = scatterSeries->zValue();
            }
#endif

#ifdef USE_SPLINEGRAPH
            if (auto splineSeries = qobject_cast<QSplineSeries *>(series)) {
                m_pointRenderer->handlePolish(splineSeries);
                if (splineSeries->zValue() > highestPointZ)
                    highestPointZ = splineSeries->zValue();
            }
#endif
        }
#endif

#ifdef USE_PIEGRAPH
        if (m_pieRenderer) {
            if (auto pieSeries = qobject_cast<QPieSeries *>(series)) {
                m_pieRenderer->handlePolish(pieSeries);
                if (pieSeries->zValue() > highestPieZ)
                    highestPieZ = pieSeries->zValue();
            }
        }
#endif

#ifdef USE_AREAGRAPH
        if (m_areaRenderer) {
            if (auto areaSeries = qobject_cast<QAreaSeries *>(series)) {
                m_areaRenderer->handlePolish(areaSeries);
                if (areaSeries->zValue() > highestAreaZ)
                    highestAreaZ = areaSeries->zValue();
            }
        }
#endif

#ifdef USE_CUSTOMGRAPH
        if (m_customRenderer) {
            if (auto customSeries = qobject_cast<QCustomSeries *>(series)) {
                m_customRenderer->handlePolish(customSeries);
                if (customSeries->zValue() > highestCustomZ)
                    highestCustomZ = customSeries->zValue();
            }
        }
#endif
    }

#ifdef USE_BARGRAPH
    if (m_barsRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[0];
        m_barsRenderer->afterPolish(cleanupSeriesList);
        if (highestBarsZ > -std::numeric_limits<float>::max())
            m_barsRenderer->setZ(highestBarsZ);
    }
#endif
#ifdef USE_POINTS
    if (m_pointRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[1];
        m_pointRenderer->afterPolish(cleanupSeriesList);
        if (highestPointZ > -std::numeric_limits<float>::max())
            m_pointRenderer->setZ(highestPointZ);
    }
#endif
#ifdef USE_AREAGRAPH
    if (m_areaRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[2];
        m_areaRenderer->afterPolish(cleanupSeriesList);
        if (highestAreaZ > -std::numeric_limits<float>::max())
            m_areaRenderer->setZ(highestAreaZ);
    }
#endif
#ifdef USE_PIEGRAPH
    if (m_pieRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[3];
        m_pieRenderer->afterPolish(cleanupSeriesList);
        if (highestPieZ > -std::numeric_limits<float>::max())
            m_pieRenderer->setZ(highestPieZ);
    }
#endif
#ifdef USE_CUSTOMGRAPH
    if (m_customRenderer) {
        auto &cleanupSeriesList = m_cleanupSeriesList[4];
        m_customRenderer->afterPolish(cleanupSeriesList);
        if (highestCustomZ > -std::numeric_limits<float>::max())
            m_customRenderer->setZ(highestCustomZ);
    }
#endif
}

void QGraphsView::polishAndUpdate()
{
    polish();
    update();
}

// ***** Static QQmlListProperty methods *****

/*!
    \qmlproperty list GraphsView::seriesList

    List of series that are rendered by the GraphsView. Filled automatically
    with the series type children of the GraphsView.

    This is the default property, so child elements are automatically added
    into the series list.
    \sa BarSeries, LineSeries, ScatterSeries
*/
QQmlListProperty<QObject> QGraphsView::seriesList()
{
    return QQmlListProperty<QObject>(this, this,
                                          &QGraphsView::appendSeriesFunc,
                                          &QGraphsView::countSeriesFunc,
                                          &QGraphsView::atSeriesFunc,
                                          &QGraphsView::clearSeriesFunc);
}

void QGraphsView::appendSeriesFunc(QQmlListProperty<QObject> *list, QObject *series)
{
    reinterpret_cast<QGraphsView *>(list->data)->addSeries(series);
}

qsizetype QGraphsView::countSeriesFunc(QQmlListProperty<QObject> *list)
{
    return reinterpret_cast<QGraphsView *>(list->data)->getSeriesList().size();
}

QObject *QGraphsView::atSeriesFunc(QQmlListProperty<QObject> *list, qsizetype index)
{
    return reinterpret_cast<QGraphsView *>(list->data)->getSeriesList().at(index);
}

void QGraphsView::clearSeriesFunc(QQmlListProperty<QObject> *list)
{
    QGraphsView *declItems = reinterpret_cast<QGraphsView *>(list->data);
    QList<QObject *> realList = declItems->getSeriesList();
    qsizetype count = realList.size();
    for (int i = 0; i < count; i++)
        declItems->removeSeries(realList.at(i));
}

/*!
    \qmlproperty GraphsTheme GraphsView::theme
    The theme used by the graph. Determines coloring,
    axis lines, fonts etc. If theme has not been set,
    the default theme is used.
*/
QGraphsTheme *QGraphsView::theme() const
{
    return m_theme;
}

void QGraphsView::setTheme(QGraphsTheme *newTheme)
{
    if (m_theme == newTheme) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "theme is already set to:" << newTheme;
        return;
    }

    if (m_theme)
        QObject::disconnect(m_theme, nullptr, this, nullptr);

    m_theme = newTheme;

    if (!m_theme) {
        m_theme = m_defaultTheme;
        m_theme->resetColorTheme();
    }

    QObject::connect(m_theme, &QGraphsTheme::update, this, &QGraphsView::polishAndUpdate);
    emit themeChanged();
    polishAndUpdate();
}

/*!
    \qmlproperty real GraphsView::marginTop
    The amount of empty space on the top of the graph.
    By default, the margin is 20.
*/
qreal QGraphsView::marginTop() const
{
    return m_marginTop;
}

void QGraphsView::setMarginTop(qreal newMarginTop)
{
    if (QtPrivate::fuzzyCompare(m_marginTop, newMarginTop)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newMarginTop);
        return;
    }
    m_marginTop = newMarginTop;
    updateComponentSizes();
    polishAndUpdate();
    emit marginTopChanged();
}

/*!
    \qmlproperty real GraphsView::marginBottom
    The amount of empty space on the bottom of the graph.
    By default, the margin is 20.
*/
qreal QGraphsView::marginBottom() const
{
    return m_marginBottom;
}

void QGraphsView::setMarginBottom(qreal newMarginBottom)
{
    if (QtPrivate::fuzzyCompare(m_marginBottom, newMarginBottom)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newMarginBottom);
        return;
    }
    m_marginBottom = newMarginBottom;
    updateComponentSizes();
    polishAndUpdate();
    emit marginBottomChanged();
}

/*!
    \qmlproperty real GraphsView::marginLeft
    The amount of empty space on the left of the graph.
    By default, the margin is 20.
*/
qreal QGraphsView::marginLeft() const
{
    return m_marginLeft;
}

void QGraphsView::setMarginLeft(qreal newMarginLeft)
{
    if (QtPrivate::fuzzyCompare(m_marginLeft, newMarginLeft)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newMarginLeft);
        return;
    }
    m_marginLeft = newMarginLeft;
    updateComponentSizes();
    polishAndUpdate();
    emit marginLeftChanged();
}

/*!
    \qmlproperty real GraphsView::marginRight
    The amount of empty space on the right of the graph.
    By default, the margin is 20.
*/
qreal QGraphsView::marginRight() const
{
    return m_marginRight;
}

void QGraphsView::setMarginRight(qreal newMarginRight)
{
    if (QtPrivate::fuzzyCompare(m_marginRight, newMarginRight)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newMarginRight);
        return;
    }
    m_marginRight = newMarginRight;
    updateComponentSizes();
    polishAndUpdate();
    emit marginRightChanged();
}

/*!
    \property QGraphsView::clipPlotArea
    \since 6.10
    \brief Controls whether graph items should be clipped
    if they go outside of a plot area. The default value is \c true.

    \sa QGraphsView::plotArea
*/
/*!
    \qmlproperty bool GraphsView::clipPlotArea
    \since 6.10
    Controls whether graph items should be clipped
    if they go outside of a plot area. The default value is \c true.

    \sa plotArea
*/
bool QGraphsView::clipPlotArea() const
{
    return m_clipPlotArea;
}

void QGraphsView::setClipPlotArea(bool enabled)
{
    if (m_clipPlotArea == enabled) {
        qCDebug(lcViewProperties2D, "QGraphsView::setClipPlotArea is already set to %d",
                 enabled);
        return;
    }

    m_clipPlotArea = enabled;
    emit clipPlotAreaChanged();
#ifdef USE_POINTS
    if (m_pointRenderer)
        m_pointRenderer->setClip(m_clipPlotArea);
#endif
#ifdef USE_AREAGRAPH
    if (m_areaRenderer)
        m_areaRenderer->setClip(m_clipPlotArea);
#endif
#ifdef USE_PIEGRAPH
    if (m_pieRenderer)
        m_pieRenderer->setClip(m_clipPlotArea);
#endif
#ifdef USE_BARGRAPH
    if (m_barsRenderer)
        m_barsRenderer->setClip(m_clipPlotArea);
#endif
#ifdef USE_CUSTOMGRAPH
    if (m_customRenderer)
        m_customRenderer->setClip(m_clipPlotArea);
#endif
}

/*!
    \property QGraphsView::plotArea
    \since 6.9
    \brief The rectangle within which the graph is drawn.

    This is the QGraphsView area minus axis areas and margins.
    \sa marginTop, marginBottom, marginLeft, marginRight
*/
/*!
    \qmlproperty rect GraphsView::plotArea
    \since 6.9
    The rectangle within which the graph is drawn.
    This is the GraphsView area minus axis areas and margins.
    \sa marginTop, marginBottom, marginLeft, marginRight
*/
QRectF QGraphsView::plotArea() const
{
    return m_plotArea;
}

void QGraphsView::updateAxisAreas()
{
    if (m_axisX && !m_axisX->isVisible()) {
        m_axisXLabelsMargin = 0;
        m_axisTickersHeight = 0;
        m_axisLabelsHeight = 0;
    } else {
        m_axisTickersHeight = m_defaultAxisTickersHeight;
        m_axisLabelsHeight = m_defaultAxisLabelsHeight;
        m_axisXLabelsMargin = m_defaultAxisXLabelsMargin;
    }

    if (m_axisY && !m_axisY->isVisible()) {
        m_axisTickersWidth = 0;
        m_axisLabelsWidth = 0;
        m_axisYLabelsMargin = 0;
    } else {
        m_axisLabelsWidth = m_defaultAxisLabelsWidth;
        m_axisTickersWidth = m_defaultAxisTickersWidth;
        m_axisYLabelsMargin = m_defaultAxisYLabelsMargin;
    }

    QRectF r = { m_marginLeft,
                 m_marginTop,
                 width() - m_marginLeft - m_marginRight,
                 height() - m_marginTop - m_marginBottom };
    m_axisHeight = m_axisLabelsHeight + m_axisXLabelsMargin + m_axisTickersHeight;
    m_axisWidth = m_axisLabelsWidth + m_axisYLabelsMargin + m_axisTickersWidth;

    int xCount = 0;
    int yCount = 0;
    int topCount = 0;
    int leftCount = 0;
    int xTitleCount = 0;
    int yTitleCount = 0;
    int topTitleCount = 0;
    int leftTitleCount = 0;

    calculateAxisCounts(&xCount,
                        &yCount,
                        &leftCount,
                        &topCount,
                        &xTitleCount,
                        &yTitleCount,
                        &leftTitleCount,
                        &topTitleCount);

    m_x1AxisArea = {r.x(),
                    r.y() + r.height() + m_axisHeight * (2 - xCount)
                        + m_axisTitleMargin * (2 - xTitleCount) - m_axisHeight * (2 - topCount)
                        - m_axisTitleMargin * (2 - topTitleCount),
                    r.width() - m_axisWidth * yCount - m_axisTitleMargin * yTitleCount,
                    m_axisHeight};
    m_x1AxisLabelsArea = { m_x1AxisArea.x(),
                         m_x1AxisArea.y() + m_axisTickersHeight + m_axisXLabelsMargin,
                         m_x1AxisArea.width(),
                         m_axisTickersHeight };
    m_x1AxisTickersArea = { m_x1AxisArea.x(),
                          m_x1AxisArea.y(),
                          m_x1AxisArea.width(),
                          m_axisTickersHeight };

    m_x2AxisArea = {r.x(),
                    r.y(),
                    r.width() - m_axisWidth * yCount - m_axisTitleMargin * yTitleCount,
                    m_axisHeight};
    m_x2AxisLabelsArea = { m_x2AxisArea.x(),
                          m_x2AxisArea.y(),
                          m_x2AxisArea.width(),
                          m_axisLabelsHeight };
    m_x2AxisTickersArea = { m_x2AxisArea.x(),
                           m_x2AxisArea.y() + m_axisLabelsHeight + m_axisXLabelsMargin,
                           m_x2AxisArea.width(),
                           m_axisTickersHeight };

    m_y1AxisArea = {r.x(),
                    r.y(),
                    m_axisWidth,
                    r.height() - m_axisHeight * xCount - m_axisTitleMargin * xTitleCount};
    m_y1AxisLabelsArea = { m_y1AxisArea.x(),
                          m_y1AxisArea.y(),
                          m_axisLabelsWidth,
                          m_y1AxisArea.height() };
    m_y1AxisTickersArea = { m_y1AxisArea.x() + m_axisLabelsWidth + m_axisYLabelsMargin,
                           m_y1AxisArea.y(),
                           m_axisTickersWidth,
                           m_y1AxisArea.height() };

    m_y2AxisArea = {r.x() + r.width() + m_axisWidth * (2 - yCount)
                        + m_axisTitleMargin * (2 - yTitleCount) - m_axisWidth * (2 - leftCount)
                        - m_axisTitleMargin * (2 - leftTitleCount),
                    r.y(),
                    m_axisWidth,
                    r.height() - m_axisHeight * xCount - m_axisTitleMargin * xTitleCount};
    m_y2AxisLabelsArea = { m_y2AxisArea.x() + m_axisTickersWidth + m_axisYLabelsMargin,
                          m_y2AxisArea.y(),
                          m_axisLabelsWidth,
                          m_y2AxisArea.height() };
    m_y2AxisTickersArea = { m_y2AxisArea.x(),
                           m_y2AxisArea.y(),
                           m_axisTickersWidth,
                           m_y2AxisArea.height() };
}

void QGraphsView::updatePlotArea()
{
    // When axis are in left & bottom
    qreal x = m_marginLeft;
    qreal y = m_marginTop;
    qreal w = width() - x - m_marginRight;
    qreal h = height() - y - m_marginBottom;

    int xCount = 0;
    int yCount = 0;
    int topCount = 0;
    int leftCount = 0;
    int xTitleCount = 0;
    int yTitleCount = 0;
    int topTitleCount = 0;
    int leftTitleCount = 0;

    calculateAxisCounts(&xCount,
                        &yCount,
                        &leftCount,
                        &topCount,
                        &xTitleCount,
                        &yTitleCount,
                        &leftTitleCount,
                        &topTitleCount);

    y += m_axisHeight * topCount + m_axisTitleMargin * topTitleCount;
    x += m_axisWidth * leftCount + m_axisTitleMargin * leftTitleCount;
    h -= m_axisHeight * xCount + m_axisTitleMargin * xTitleCount;
    w -= m_axisWidth * yCount + m_axisTitleMargin * yTitleCount;

    w = qMax(w, 0.0);
    h = qMax(h, 0.0);
    QRectF plotArea = QRectF(x, y, w, h);
    if (plotArea != m_plotArea) {
        m_plotArea = plotArea;
        emit plotAreaChanged();
    }
}

/*!
    \property QGraphsView::axisX
    \brief X-axis of this view.

    The x-axis used for the series inside this view.

    \note Setting the same axis to multiple QGraphsViews is not supported.
*/
/*!
    \qmlproperty AbstractAxis GraphsView::axisX
    The x-axis used for the series inside this view.
    \sa axisY

    \note Setting the same axis to multiple GraphsViews is not supported.
*/

QAbstractAxis *QGraphsView::axisX() const
{
    return m_axisX;
}

void QGraphsView::setAxisX(QAbstractAxis *axis)
{
    if (m_axisX == axis) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    if (m_axisX)
        removeAxis(m_axisX);
    m_axisX = axis;
    if (axis) {
        if (axis->alignment() != Qt::AlignBottom && axis->alignment() != Qt::AlignTop)
            axis->setAlignment(Qt::AlignBottom);
        addAxis(axis);
    }
    updateComponentSizes();
    emit axisXChanged();
    update();
    polishAndUpdate();
}

/*!
    \property QGraphsView::axisY
    \brief Y-axis of this view.

    The y-axis used for the series inside this view.

    \note Setting the same axis to multiple QGraphsViews is not supported.
*/
/*!
    \qmlproperty AbstractAxis GraphsView::axisY
    The y-axis used for the series inside this view.
    \sa axisX

    \note Setting the same axis to multiple GraphsViews is not supported.
*/

QAbstractAxis *QGraphsView::axisY() const
{
    return m_axisY;
}

void QGraphsView::setAxisY(QAbstractAxis *axis)
{
    if (m_axisY == axis) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    if (m_axisY)
        removeAxis(m_axisY);
    m_axisY = axis;
    if (axis) {
        if (axis->alignment() != Qt::AlignLeft && axis->alignment() != Qt::AlignRight)
            axis->setAlignment(Qt::AlignLeft);
        addAxis(axis);
    }
    updateComponentSizes();
    emit axisYChanged();
    update();
    polishAndUpdate();
}

/*!
    \property QGraphsView::orientation
    \brief Orientation of the GraphsView.

    Determines the orientation of the QGraphsView. When the orientation is
    \l {Qt::Horizontal}{Qt::Horizontal}, \l axisX and \l axisY will switch the
    positions so that \l axisX is rendered vertically and \l axisY horizontally.
    This property is currently used by the \l QBarSeries.
    The default value is \l {Qt::Vertical}{Qt::Vertical}.
*/
/*!
    \qmlproperty Qt.Orientation GraphsView::orientation
    Determines the orientation of the GraphsView. When the orientation is
    \l {Qt::Horizontal}{Qt.Horizontal}, \l axisX and \l axisY will switch the
    positions so that \l axisX is rendered vertically and \l axisY horizontally.
    This property is currently used by the \l BarSeries.
    The default value is \l {Qt::Vertical}{Qt.Vertical}.
*/
Qt::Orientation QGraphsView::orientation() const
{
    return m_orientation;
}

void QGraphsView::setOrientation(Qt::Orientation newOrientation)
{
    if (m_orientation == newOrientation) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newOrientation;
        return;
    }
    m_orientation = newOrientation;
    emit orientationChanged();
    update();
    updateComponentSizes();
    polishAndUpdate();
}

/*
    mapX and mapY functions are used to map custom series data values
    to the real pixel values of the GraphsView
*/
qreal QGraphsView::mapX(QCustomSeries *series, qreal x)
{
    return m_customRenderer->mapX(m_axisRenderer, series, x);
}

qreal QGraphsView::mapY(QCustomSeries *series, qreal y)
{
    return m_customRenderer->mapY(m_axisRenderer, series, y);
}

CustomRenderer *QGraphsView::customRenderer() const
{
    return m_customRenderer;
}

/*!
    \enum QGraphsView::ZoomStyle
    This enum value describes the zoom style of the graph:

    \value None
        Zooming is disabled.
    \value Center
        Pinch zoom and mouse wheel zoom towards the center of the graph view.
*/

/*!
    \property QGraphsView::zoomStyle
    \brief Zoom style of the GraphsView.

    Determines the zoom style of the QGraphsView. Zooming works by
    manipulating the QValueAxis zoom property. The default value
    is \c {QGraphsView::ZoomStyle::None}.
*/
/*!
    \qmlproperty enumeration GraphsView::zoomStyle
    Determines the zoom style of the GraphsView. Zooming works by
    manipulating the ValueAxis zoom property. The default value
    is \c {GraphsView.ZoomStyle.None}.

    \value GraphsView.ZoomStyle.None
        Zooming is disabled.
    \value GraphsView.ZoomStyle.Center
        Pinch zoom and mouse wheel zoom towards the center of the graph view.

*/
QGraphsView::ZoomStyle QGraphsView::zoomStyle() const
{
    return m_zoomStyle;
}

void QGraphsView::setZoomStyle(ZoomStyle newZoomStyle)
{
    if (m_zoomStyle == newZoomStyle) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newZoomStyle;
        return;
    }
    m_zoomStyle = newZoomStyle;
    emit zoomStyleChanged();
}

/*!
    \enum QGraphsView::PanStyle
    This enum value describes the pan style of the graph:

    \value None
        Panning is disabled.
    \value Drag
        Mouse and touch drag pan the view around.
*/

/*!
    \property QGraphsView::panStyle
    \brief Pan style of the GraphsView.

    Determines the pan style of the QGraphsView. Panning works by
    manipulating the pan property of a QValueAxis.
    The default value is \c {QGraphsView::PanStyle::None}.
*/
/*!
    \qmlproperty enumeration GraphsView::panStyle
    Determines the pan style of the GraphsView. Panning works by
    manipulating the pan property of a ValueAxis.
    The default value is \c {GraphsView.PanStyle.None}.

    \value GraphsView.PanStyle.None
        Panning is disabled.
    \value GraphsView.PanStyle.Drag
        Mouse and touch drag pan the view around.
*/
QGraphsView::PanStyle QGraphsView::panStyle() const
{
    return m_panStyle;
}

void QGraphsView::setPanStyle(PanStyle newPanStyle)
{
    if (m_panStyle == newPanStyle) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newPanStyle;
        return;
    }
    m_panStyle = newPanStyle;
    emit panStyleChanged();
}

/*!
    \property QGraphsView::zoomAreaEnabled
    \brief Enables zoom area

    Zoom area changes mouse and touch dragging to draw a box determined
    by \c zoomAreaDelegate. Upon release the graph QValueAxis zoom and pan
    properties are changed so that the view covers only the area intersected
    by the drawn box.
    \sa zoomAreaDelegate
*/
/*!
    \qmlproperty bool GraphsView::zoomAreaEnabled
    Zoom area changes mouse and touch dragging to draw a box determined
    by \c zoomAreaDelegate. Upon release the graph ValueAxis zoom and pan
    properties are changed so that the view covers only the area intersected
    by the drawn box.
    \sa zoomAreaDelegate
*/
bool QGraphsView::zoomAreaEnabled() const
{
    return m_zoomAreaEnabled;
}

void QGraphsView::setZoomAreaEnabled(bool newZoomAreaEnabled)
{
    if (m_zoomAreaEnabled == newZoomAreaEnabled) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newZoomAreaEnabled;
        return;
    }
    m_zoomAreaEnabled = newZoomAreaEnabled;
    emit zoomAreaEnabledChanged();
}

/*!
    \property QGraphsView::zoomAreaDelegate
    \brief Zoom area visual delegate

    Determines the QML element that is drawn when the user performs a drag
    motion to zoom in to an area.
*/
/*!
    \qmlproperty Component GraphsView::zoomAreaDelegate
    Determines the QML element that is drawn when the user performs a drag
    motion to zoom in to an area.
*/
QQmlComponent *QGraphsView::zoomAreaDelegate() const
{
    return m_zoomAreaDelegate;
}

void QGraphsView::setZoomAreaDelegate(QQmlComponent *newZoomAreaDelegate)
{
    if (m_zoomAreaDelegate == newZoomAreaDelegate) {
        qCDebug(lcViewProperties2D) << __FUNCTION__
            << "value is already set to:" << newZoomAreaDelegate;
        return;
    }
    m_zoomAreaDelegate = newZoomAreaDelegate;

    if (m_zoomAreaDelegate) {
        m_zoomAreaItem = qobject_cast<QQuickItem *>(
            m_zoomAreaDelegate->create(m_zoomAreaDelegate->creationContext()));
        m_zoomAreaItem->setParent(this);
        m_zoomAreaItem->setParentItem(this);
        m_zoomAreaItem->setVisible(false);
    }

    emit zoomAreaDelegateChanged();
}

/*!
    \property QGraphsView::zoomSensitivity
    \brief Zoom value change sensitivity

    Determines how fast zoom value changes while zooming.
*/
/*!
    \qmlproperty real GraphsView::zoomSensitivity
    Determines how fast zoom value changes while zooming.
*/
qreal QGraphsView::zoomSensitivity() const
{
    return m_zoomSensitivity;
}

void QGraphsView::setZoomSensitivity(qreal newZoomSensitivity)
{
    if (QtPrivate::fuzzyCompare(m_zoomSensitivity, newZoomSensitivity)) {
        qCDebug(lcViewProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newZoomSensitivity);
        return;
    }
    m_zoomSensitivity = newZoomSensitivity;
    emit zoomSensitivityChanged();
}

void QGraphsView::calculateAxisCounts(int *xCount, int *yCount, int *leftCount, int *topCount,
                                      int *xTitleCount, int *yTitleCount, int *leftTitleCount, int *topTitleCount)
{
    if (axisY()) {
        (*yCount)++;
        if (axisY()->isTitleVisible() && !axisY()->titleText().isEmpty())
            (*yTitleCount)++;

        if (axisY()->alignment() == Qt::AlignLeft) {
            (*leftCount)++;
            if (axisY()->isTitleVisible() && !axisY()->titleText().isEmpty())
                (*leftTitleCount)++;
        }
    }

    if (axisX()) {
        (*xCount)++;
        if (axisX()->isTitleVisible() && !axisX()->titleText().isEmpty())
            (*xTitleCount)++;

        if (axisX()->alignment() == Qt::AlignTop) {
            (*topCount)++;
            if (axisX()->isTitleVisible() && !axisX()->titleText().isEmpty())
                (*topTitleCount)++;
        }
    }

    QSet<QAbstractAxis *> countedAxes;

    for (auto&& s : m_seriesList) {
        if (auto series = qobject_cast<QAbstractSeries *>(s)) {
            QAbstractAxis *seriesYAxis = series->axisY();
            if (seriesYAxis && seriesYAxis != axisY() && !countedAxes.contains(seriesYAxis)) {
                countedAxes.insert(seriesYAxis);
                (*yCount)++;
                if (seriesYAxis->isTitleVisible() && !seriesYAxis->titleText().isEmpty())
                    (*yTitleCount)++;

                if (seriesYAxis->alignment() == Qt::AlignLeft) {
                    (*leftCount)++;
                    if (seriesYAxis->isTitleVisible() && !seriesYAxis->titleText().isEmpty())
                        (*leftTitleCount)++;
                }
            }

            QAbstractAxis *seriesXAxis = series->axisX();
            if (seriesXAxis && seriesXAxis != axisX() && !countedAxes.contains(seriesXAxis)) {
                countedAxes.insert(seriesXAxis);
                (*xCount)++;
                if (seriesXAxis->isTitleVisible() && !seriesXAxis->titleText().isEmpty())
                    (*xTitleCount)++;

                if (seriesXAxis->alignment() == Qt::AlignTop) {
                    (*topCount)++;
                    if (seriesXAxis->isTitleVisible() && !seriesXAxis->titleText().isEmpty())
                        (*topTitleCount)++;
                }
            }
        }
    }

    if (m_orientation == Qt::Horizontal) {
        qSwap((*xCount), (*yCount));
        qSwap((*leftCount), (*topCount));
        qSwap((*xTitleCount), (*yTitleCount));
        qSwap((*leftTitleCount), (*topTitleCount));
    }

    if ((*xTitleCount) > 0) {
        (*xTitleCount)--;
        if ((*xTitleCount) > 0)
            (*xTitleCount)--;
    }

    if ((*yTitleCount) > 0) {
        (*yTitleCount)--;
        if ((*yTitleCount) > 0)
            (*yTitleCount)--;
    }

    if ((*leftTitleCount) > 0)
        (*leftTitleCount)--;

    if ((*topTitleCount) > 0)
        (*topTitleCount)--;
}

int QGraphsView::getSeriesRendererIndex(QAbstractSeries *series)
{
    int index = 0;
    if (series) {
        switch (QAbstractSeriesPrivate::get(series)->type()) {
        case QAbstractSeries::SeriesType::Bar:
            index = 0;
            break;
        case QAbstractSeries::SeriesType::Scatter:
        case QAbstractSeries::SeriesType::Line:
        case QAbstractSeries::SeriesType::Spline:
            index = 1;
            break;
        case QAbstractSeries::SeriesType::Area:
            index = 2;
            break;
        case QAbstractSeries::SeriesType::Pie:
            index = 3;
            break;
        case QAbstractSeries::SeriesType::Custom:
            index = 4;
            break;
        }
    }
    return index;
}

QT_END_NAMESPACE

#include "moc_qgraphsview_p.cpp"
