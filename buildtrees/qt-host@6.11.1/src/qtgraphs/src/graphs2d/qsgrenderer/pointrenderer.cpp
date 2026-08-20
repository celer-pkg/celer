// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifdef USE_LINEGRAPH
#include <QtGraphs/qlineseries.h>
#endif
#ifdef USE_SCATTERGRAPH
#include <QtGraphs/qscatterseries.h>
#endif
#ifdef USE_SPLINEGRAPH
#include <QtGraphs/qsplineseries.h>
#endif
#include <QtQuick/private/qquickdraghandler_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <private/axisrenderer_p.h>
#include <private/pointrenderer_p.h>
#include <private/qabstractseries_p.h>
#include <private/qgraphsview_p.h>
#include <private/qxyseries_p.h>
#include <private/charthelpers_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
              "class PointRenderer;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererAfterPolish_entry, int cleanupSeriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererAfterPolish_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateScatterSeries_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateScatterSeries_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateLineSeries_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateLineSeries_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateSplineSeries_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DPointRendererUpdateSplineSeries_exit);

static const char *TAG_POINT_COLOR = "pointColor";
static const char *TAG_POINT_BORDER_COLOR = "pointBorderColor";
static const char *TAG_POINT_BORDER_WIDTH = "pointBorderWidth";
static const char *TAG_POINT_SELECTED_COLOR = "pointSelectedColor";
static const char *TAG_POINT_SELECTED = "pointSelected";
static const char *TAG_POINT_VALUE_X = "pointValueX";
static const char *TAG_POINT_VALUE_Y = "pointValueY";
static const char *TAG_POINT_INDEX = "pointIndex";

PointRenderer::PointRenderer(QGraphsView *graph, bool clipPlotArea)
    : QQuickItem(graph)
    , m_graph(graph)
{
    setFlag(QQuickItem::ItemHasContents);
    setClip(clipPlotArea);

    m_shape.setParentItem(this);
    m_shape.setPreferredRendererType(QQuickShape::CurveRenderer);

    const QString qmlData = QLatin1StringView(R"QML(
        import QtQuick;

        Rectangle {
            property bool pointSelected
            property color pointColor
            property color pointBorderColor
            property color pointSelectedColor
            property real pointBorderWidth
            color: pointSelected ? pointSelectedColor : pointColor
            border.color: pointBorderColor
            border.width: pointBorderWidth
            width: %1
            height: %1
        }
    )QML").arg(QString::number((int) defaultSize()));
    m_tempMarker = new QQmlComponent(qmlEngine(m_graph), this);
    m_tempMarker->setData(qmlData.toUtf8(), QUrl());

    m_tapHandler = new QQuickTapHandler(this);
    connect(m_tapHandler, &QQuickTapHandler::singleTapped,
            this, &PointRenderer::onSingleTapped);
    connect(m_tapHandler, &QQuickTapHandler::doubleTapped,
            this, &PointRenderer::onDoubleTapped);
    connect(m_tapHandler, &QQuickTapHandler::pressedChanged,
            this, &PointRenderer::onPressedChanged);
}

PointRenderer::~PointRenderer()
{
    qDeleteAll(m_groups);
}

void PointRenderer::resetShapePathCount()
{
    m_currentShapePathIndex = 0;
}

qreal PointRenderer::defaultSize(QXYSeries *series)
{
    qreal size = 16.0;
    if (series != nullptr) {
#ifdef USE_LINEGRAPH
        if (auto line = qobject_cast<QLineSeries *>(series))
            size = qMax(size, line->width());
#endif
#if defined(USE_LINEGRAPH) && defined(USE_SPLINEGRAPH)
        else
#endif
#ifdef USE_SPLINEGRAPH
            if (auto spline = qobject_cast<QSplineSeries *>(series))
            size = qMax(size, spline->width());
#endif
    }
    return size;
}

void PointRenderer::calculateRenderCoordinates(AxisRenderer *axisRenderer,
                                               QAbstractSeries *series,
                                               qreal origX,
                                               qreal origY,
                                               qreal *renderX,
                                               qreal *renderY)
{
    auto &axisX = axisRenderer->getAxisX(series);
    auto &axisY = axisRenderer->getAxisY(series);

    if (m_graph->orientation() != Qt::Vertical) {
        std::swap(origX, origY);
        origY = axisY.maxValue - origY;
    }

    auto flipX = axisX.maxValue < axisX.minValue ? -1 : 1;
    auto flipY = axisY.maxValue < axisY.minValue ? -1 : 1;

    *renderX = m_areaWidth * flipX * origX * m_maxHorizontal - m_horizontalOffset;
    *renderY = m_areaHeight - m_areaHeight * flipY * origY * m_maxVertical
               + m_verticalOffset;
}

void PointRenderer::reverseRenderCoordinates(AxisRenderer *axisRenderer,
                                             QAbstractSeries *series,
                                             qreal renderX,
                                             qreal renderY,
                                             qreal *origX,
                                             qreal *origY)
{
    auto &axisX = axisRenderer->getAxisX(series);
    auto &axisY = axisRenderer->getAxisY(series);

    if (m_graph->orientation() != Qt::Vertical) {
        std::swap(renderX, renderY);
        renderY = m_areaHeight - renderY;
    }

    auto flipX = axisX.maxValue < axisX.minValue ? -1 : 1;
    auto flipY = axisY.maxValue < axisY.minValue ? -1 : 1;

    *origX = (renderX + m_horizontalOffset) / (m_areaWidth * flipX * m_maxHorizontal);
    *origY = (renderY - m_areaHeight - m_verticalOffset)
             / (-1 * m_areaHeight * flipY * m_maxVertical);
}

PointRenderer::SeriesStyle PointRenderer::getSeriesStyle(PointGroup *group)
{
    auto theme = m_graph->theme();

    const auto &seriesColors = theme->seriesColors();
    const auto &borderColors = theme->borderColors();

    qsizetype index = group->colorIndex % seriesColors.size();
    QColor color = group->series->color().alpha() != 0 ? group->series->color() : seriesColors.at(index);
    color.setAlpha(color.alpha() * group->series->opacity());

    QColor selectedColor = group->series->selectedColor().alpha() != 0
                               ? group->series->selectedColor()
                               : m_graph->theme()->singleHighlightColor();
    selectedColor.setAlpha(selectedColor.alpha() * group->series->opacity());

    index = group->colorIndex % borderColors.size();
    QColor borderColor = borderColors.at(index);
    borderColor.setAlpha(borderColor.alpha() * group->series->opacity());
    qreal borderWidth = theme->borderWidth();

    return {
        color,
        selectedColor,
        borderColor,
        borderWidth
    };
}

void PointRenderer::updatePointDelegate(
    QXYSeries *series, PointGroup *group, qsizetype pointIndex, qreal x, qreal y)
{
    const auto style = getSeriesStyle(group);

    auto marker = group->markers[pointIndex];
    auto &rect = group->rects[pointIndex];

    if (marker->property(TAG_POINT_SELECTED).isValid())
        marker->setProperty(TAG_POINT_SELECTED, series->isPointSelected(pointIndex));
    if (marker->property(TAG_POINT_COLOR).isValid())
        marker->setProperty(TAG_POINT_COLOR, style.color);
    if (marker->property(TAG_POINT_BORDER_COLOR).isValid())
        marker->setProperty(TAG_POINT_BORDER_COLOR, style.borderColor);
    if (marker->property(TAG_POINT_BORDER_WIDTH).isValid())
        marker->setProperty(TAG_POINT_BORDER_WIDTH, style.borderWidth);
    if (marker->property(TAG_POINT_SELECTED_COLOR).isValid())
        marker->setProperty(TAG_POINT_SELECTED_COLOR, style.selectedColor);
    const auto point = series->points().at(pointIndex);
    if (marker->property(TAG_POINT_VALUE_X).isValid())
        marker->setProperty(TAG_POINT_VALUE_X, point.x());
    if (marker->property(TAG_POINT_VALUE_Y).isValid())
        marker->setProperty(TAG_POINT_VALUE_Y, point.y());
    if (marker->property(TAG_POINT_INDEX).isValid())
        marker->setProperty(TAG_POINT_INDEX, pointIndex);

    marker->setX(x - marker->width() / 2.0);
    marker->setY(y - marker->height() / 2.0);
    marker->setVisible(isValidValue(x) && isValidValue(y));

    rect = QRectF(x - marker->width() / 2.0,
                  y - marker->height() / 2.0,
                  marker->width(),
                  marker->height());
}

void PointRenderer::hidePointDelegates(QXYSeries *series)
{
    auto *group = m_groups.value(series);
    if (group->currentMarker) {
        for (int i = 0; i < group->markers.size(); ++i) {
            auto *marker = group->markers[i];
            marker->setVisible(false);
        }
    }
    group->rects.clear();
}

void PointRenderer::updateLegendData(QXYSeries *series, QLegendData &legendData)
{
    QList<QLegendData> legendDataList = {legendData};
    series->d_func()->setLegendData(legendDataList);
}

void PointRenderer::onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    for (auto &&group : m_groups) {
        if (!group->series->isVisible())
            continue;

        if (!group->series->isSelectable() && !group->series->isDraggable())
            continue;

        int index = 0;
        for (auto &&rect : group->rects) {
            if (rect.contains(eventPoint.position())) {
                emit group->series->clicked(group->series->at(index).toPoint());
                return;
            }
            index++;
        }
    }
}

void PointRenderer::onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    for (auto &&group : m_groups) {
        if (!group->series->isVisible())
            continue;

        if (!group->series->isSelectable() && !group->series->isDraggable())
            continue;

        int index = 0;
        for (auto &&rect : group->rects) {
            if (rect.contains(eventPoint.position())) {
                emit group->series->doubleClicked(group->series->at(index).toPoint());
                return;
            }
            index++;
        }
    }
}

void PointRenderer::onPressedChanged()
{
    if (m_tapHandler->isPressed()) {
        for (auto &&group : m_groups) {
            if (!group->series->isVisible())
                continue;

            if (!group->series->isSelectable() && !group->series->isDraggable())
                continue;

            int index = 0;
            for (auto &&rect : group->rects) {
                if (rect.contains(m_tapHandler->point().position())) {
                    m_pressedGroup = group;
                    m_pressedPointIndex = index;
                    emit group->series->pressed(m_pressedGroup->series->at(index).toPoint());
                }
                index++;
            }
        }
    } else {
        if (m_pressedGroup
            && m_pressedGroup->series->isSelectable()
            && m_pressedGroup->series->isVisible()) {
            if (m_pressedGroup->rects[m_pressedPointIndex].contains(
                    m_tapHandler->point().position())) {
                if (m_pressedGroup->series->isPointSelected(m_pressedPointIndex))
                    m_pressedGroup->series->deselectPoint(m_pressedPointIndex);
                else
                    m_pressedGroup->series->selectPoint(m_pressedPointIndex);
                m_previousDelta = QPoint(0, 0);
                emit m_pressedGroup->series->released(
                    m_pressedGroup->series->at(m_pressedPointIndex).toPoint());
            }
        }
    }
}

#ifdef USE_SCATTERGRAPH
void PointRenderer::updateScatterSeries(QScatterSeries *series, QLegendData &legendData)
{
    auto group = m_groups.value(series);
    const auto style = getSeriesStyle(group);
    Q_TRACE(QGraphs2DPointRendererUpdateScatterSeries_entry);

    if (series->isVisible()) {
        auto &&points = series->points();
        group->rects.resize(points.size());
        for (int i = 0; i < points.size(); ++i) {
            qreal x, y;
            calculateRenderCoordinates(m_graph->m_axisRenderer,
                                       series,
                                       points[i].x(),
                                       points[i].y(),
                                       &x,
                                       &y);
            y *= series->valuesMultiplier();
            if (group->currentMarker) {
                updatePointDelegate(series, group, i, x, y);
            } else {
                auto &rect = group->rects[i];
                qreal size = defaultSize(series);
                rect = QRectF(x - size / 2.0, y - size / 2.0, size, size);
            }
        }
    } else {
        hidePointDelegates(series);
    }
    Q_TRACE(QGraphs2DPointRendererUpdateScatterSeries_exit);

    legendData = { style.color, style.borderColor, series->name() };
}
#endif

#ifdef USE_LINEGRAPH
void PointRenderer::updateLineSeries(QLineSeries *series, QLegendData &legendData)
{
    auto group = m_groups.value(series);
    const auto style = getSeriesStyle(group);

    group->shapePath->setStrokeColor(style.color);
    group->shapePath->setStrokeWidth(series->width());
    group->shapePath->setFillColor(QColorConstants::Transparent);

    Qt::PenCapStyle capStyle = series->capStyle();
    if (capStyle == Qt::PenCapStyle::SquareCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::SquareCap);
    else if (capStyle == Qt::PenCapStyle::FlatCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::FlatCap);
    else if (capStyle == Qt::PenCapStyle::RoundCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::RoundCap);

    Qt::PenJoinStyle joinStyle = series->joinStyle();
    if (joinStyle == Qt::PenJoinStyle::BevelJoin)
        group->shapePath->setJoinStyle(QQuickShapePath::JoinStyle::BevelJoin);
    else if (joinStyle == Qt::PenJoinStyle::MiterJoin)
        group->shapePath->setJoinStyle(QQuickShapePath::JoinStyle::MiterJoin);
    else if (joinStyle == Qt::PenJoinStyle::RoundJoin)
        group->shapePath->setJoinStyle(QQuickShapePath::JoinStyle::RoundJoin);

    group->shapePath->setStrokeStyle(QQuickShapePath::StrokeStyle(series->strokeStyle()));
    group->shapePath->setDashOffset(series->dashOffset());
    group->shapePath->setDashPattern(series->dashPattern());

    auto &painterPath = group->painterPath;
    painterPath.clear();

    QPointF previousRenderCoordinates;

    Q_TRACE(QGraphs2DPointRendererUpdateLineSeries_entry);
    if (series->isVisible()) {
        auto &&points = series->points();
        group->rects.resize(points.size());
        for (int i = 0; i < points.size(); ++i) {
            qreal x, y;
            calculateRenderCoordinates(m_graph->m_axisRenderer,
                                       series,
                                       points[i].x(),
                                       points[i].y(),
                                       &x,
                                       &y);
            y *= series->valuesMultiplier();
            if (i == 0)
                painterPath.moveTo(x, y);
            else {
                if (!isValidValue(previousRenderCoordinates.x()) || !isValidValue(previousRenderCoordinates.y())) {
                    if (isValidValue(x) && isValidValue(y))
                        painterPath.moveTo(x, y);
                } else if (isValidValue(x) && isValidValue(y)) {
                    switch (series->lineStyle()) {
                    case QLineSeries::LineStyle::StepLeft:
                        painterPath.lineTo(x, previousRenderCoordinates.y());
                        painterPath.lineTo(x, y);
                        break;
                    case QLineSeries::LineStyle::StepRight:
                        painterPath.lineTo(previousRenderCoordinates.x(), y);
                        painterPath.lineTo(x, y);
                        break;
                    case QLineSeries::LineStyle::StepCenter: {
                        qreal half = (x + previousRenderCoordinates.x()) / 2;
                        painterPath.lineTo(half, previousRenderCoordinates.y());
                        painterPath.lineTo(half, y);
                        painterPath.lineTo(x, y);
                    } break;
                    default:
                        painterPath.lineTo(x, y);
                        break;
                    }
                }
            }
            previousRenderCoordinates = {x, y};

            if (group->currentMarker) {
                updatePointDelegate(series, group, i, x, y);
            } else {
                auto &rect = group->rects[i];
                qreal size = defaultSize(series);
                rect = QRectF(x - size / 2.0, y - size / 2.0, size, size);
            }
        }
    } else {
        hidePointDelegates(series);
    }
    group->shapePath->setPath(painterPath);
    Q_TRACE(QGraphs2DPointRendererUpdateLineSeries_exit);
    legendData = { style.color, style.borderColor, series->name() };
}
#endif

#ifdef USE_SPLINEGRAPH
void PointRenderer::updateSplineSeries(QSplineSeries *series, QLegendData &legendData)
{
    auto group = m_groups.value(series);
    const auto style = getSeriesStyle(group);

    group->shapePath->setStrokeColor(style.color);
    group->shapePath->setStrokeWidth(series->width());
    group->shapePath->setFillColor(QColorConstants::Transparent);

    Qt::PenCapStyle capStyle = series->capStyle();
    if (capStyle == Qt::PenCapStyle::SquareCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::SquareCap);
    else if (capStyle == Qt::PenCapStyle::FlatCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::FlatCap);
    else if (capStyle == Qt::PenCapStyle::RoundCap)
        group->shapePath->setCapStyle(QQuickShapePath::CapStyle::RoundCap);

    auto &painterPath = group->painterPath;
    painterPath.clear();

    if (series->isVisible()) {
        auto &&points = series->points();
        group->rects.resize(points.size());
        auto fittedPoints = series->getControlPoints();

        Q_TRACE(QGraphs2DPointRendererUpdateSplineSeries_entry);
        for (int i = 0, j = 0; i < points.size(); ++i, ++j) {
            qreal x, y;
            calculateRenderCoordinates(m_graph->m_axisRenderer,
                                       series,
                                       points[i].x(),
                                       points[i].y(),
                                       &x,
                                       &y);

            qreal valuesMultiplier = series->valuesMultiplier();
            y *= valuesMultiplier;
            if (i == 0) {
                painterPath.moveTo(x, y);
            } else {
                qreal x1, y1, x2, y2;
                calculateRenderCoordinates(m_graph->m_axisRenderer,
                                           series,
                                           fittedPoints[j - 1].x(),
                                           fittedPoints[j - 1].y(),
                                           &x1,
                                           &y1);
                calculateRenderCoordinates(m_graph->m_axisRenderer,
                                           series,
                                           fittedPoints[j].x(),
                                           fittedPoints[j].y(),
                                           &x2,
                                           &y2);

                y1 *= valuesMultiplier;
                y2 *= valuesMultiplier;
                painterPath.cubicTo(x1, y1, x2, y2, x, y);
                ++j;
            }

            if (group->currentMarker) {
                updatePointDelegate(series, group, i, x, y);
            } else {
                auto &rect = group->rects[i];
                qreal size = defaultSize(series);
                rect = QRectF(x - size / 2.0, y - size / 2.0, size, size);
            }
        }
        Q_TRACE(QGraphs2DPointRendererUpdateSplineSeries_exit);
    } else {
        hidePointDelegates(series);
    }

    group->shapePath->setPath(painterPath);
    legendData = { style.color, style.borderColor, series->name() };
}
#endif

void PointRenderer::handlePolish(QXYSeries *series)
{
    auto theme = m_graph->theme();
    if (!theme) {
        qCCritical(lcCritical2D, "Theme not found");
        return;
    }

    if (!m_graph->m_axisRenderer) {
        qCCritical(lcCritical2D, "Axis renderer not found.");
        return;
    }

    if (series->points().isEmpty()) {
        auto group = m_groups.value(series);

        if (group) {
            if (group->shapePath) {
                auto &painterPath = group->painterPath;
                painterPath.clear();
                group->shapePath->setPath(painterPath);
            }

            for (auto m : std::as_const(group->markers))
                m->deleteLater();

            group->markers.clear();
        }

        return;
    }

    if (width() <= 0 || height() <= 0)
        return;

    m_areaWidth = width();
    m_areaHeight = height();

    auto &axisX = m_graph->m_axisRenderer->getAxisX(series);
    auto &axisY = m_graph->m_axisRenderer->getAxisY(series);

    m_maxVertical = axisY.valueRange > 0 ? 1.0 / axisY.valueRange : 100.0;
    m_maxHorizontal = axisX.valueRange > 0 ? 1.0 / axisX.valueRange : 100.0;

    auto vmin = axisY.minValue > axisY.maxValue ? std::abs(axisY.minValue) : axisY.minValue;
    m_verticalOffset = (vmin / axisY.valueRange) * m_areaHeight;

    auto hmin = axisX.minValue > axisX.maxValue ? std::abs(axisX.minValue) : axisX.minValue;
    m_horizontalOffset = (hmin / axisX.valueRange) * m_areaWidth;

    if (!m_groups.contains(series)) {
        PointGroup *group = new PointGroup();
        group->series = series;
        m_groups.insert(series, group);

        if (series->type() != QAbstractSeries::SeriesType::Scatter) {
            group->shapePath = new QQuickShapePath(&m_shape);
            group->shapePath->setAsynchronous(true);
            auto data = m_shape.data();
            data.append(&data, group->shapePath);
        }
    }

    auto group = m_groups.value(series);

    if (series->type() != QAbstractSeries::SeriesType::Scatter) {
        auto data = m_shape.data();
        group->shapePath = qobject_cast<QQuickShapePath *>(data.at(&data, m_currentShapePathIndex));

        m_currentShapePathIndex++;
    }

    qsizetype pointCount = series->points().size();

    if ((series->type() == QAbstractSeries::SeriesType::Scatter) && !series->pointDelegate())
        group->currentMarker = m_tempMarker;
    else if (series->pointDelegate())
        group->currentMarker = series->pointDelegate();

    if (group->currentMarker != group->previousMarker) {
        for (auto &&marker : group->markers)
            marker->deleteLater();
        group->markers.clear();
    }
    group->previousMarker = group->currentMarker;

    if (group->currentMarker) {
        qsizetype markerCount = group->markers.size();
        if (markerCount < pointCount) {
            for (qsizetype i = markerCount; i < pointCount; ++i) {
                QQuickItem *item = qobject_cast<QQuickItem *>(
                    group->currentMarker->create(group->currentMarker->creationContext()));
                item->setParent(this);
                item->setParentItem(this);
                QQuickDragHandler *handler = new QQuickDragHandler(item);
                handler->setEnabled(series->isDraggable());
                connect(series, &QXYSeries::draggableChanged, this, [handler, series]() {
                    handler->setEnabled(series->isDraggable());
                });
                group->markers << item;
                group->dragHandlers << handler;

                connect(handler, &QQuickDragHandler::translationChanged, this, [&]() {
                    if (m_pressedGroup) {
                        float w = width();
                        float h = height();
                        double maxVertical
                            = axisY.valueRange > 0
                                  ? 1.0 / axisY.valueRange
                                  : 100.0;
                        double maxHorizontal
                            = axisX.valueRange > 0
                                  ? 1.0 / axisX.valueRange
                                  : 100.0;

                        QPoint currentDelta =
                            m_pressedGroup->dragHandlers.at(m_pressedPointIndex)
                                ->activeTranslation().toPoint();
                        QPoint delta = currentDelta - m_previousDelta;
                        m_previousDelta = currentDelta;

                        qreal deltaX = delta.x() / w / maxHorizontal;
                        qreal deltaY = -delta.y() / h / maxVertical;

                        QPointF point = m_pressedGroup->series->at(m_pressedPointIndex)
                                        + QPointF(deltaX, deltaY);
                        m_pressedGroup->series->replace(m_pressedPointIndex, point);
                    }
                });
                connect(handler, &QQuickDragHandler::grabChanged, this,
                        [&](QPointingDevice::GrabTransition transition, QEventPoint point) {
                            Q_UNUSED(point)

                            if (transition == QPointingDevice::UngrabExclusive ||
                                transition == QPointingDevice::UngrabPassive) {
                                m_previousDelta = QPoint(0, 0);
                            }
                        });
            }
        } else if (markerCount > pointCount) {
            for (qsizetype i = pointCount; i < markerCount; ++i)
                group->markers[i]->deleteLater();
            group->markers.resize(pointCount);
        }
    } else if (group->markers.size() > 0) {
        for (auto &&marker : group->markers)
            marker->deleteLater();
        group->markers.clear();
    }

    for (auto &&marker : group->markers)
        marker->setZ(group->series->zValue());

    if (group->colorIndex < 0) {
        group->colorIndex = m_graph->graphSeriesCount();
        m_graph->setGraphSeriesCount(group->colorIndex + 1);
    }

    QLegendData legendData;
#ifdef USE_SCATTERGRAPH
    if (auto scatter = qobject_cast<QScatterSeries *>(series))
        updateScatterSeries(scatter, legendData);
#endif
#if defined(USE_SCATTERGRAPH) && defined(USE_LINEGRAPH)
    else
#endif
#ifdef USE_LINEGRAPH
        if (auto line = qobject_cast<QLineSeries *>(series))
        updateLineSeries(line, legendData);
#endif
#if defined(USE_LINEGRAPH) && defined(USE_SPLINEGRAPH)
    else
#endif
#ifdef USE_SPLINEGRAPH
        if (auto spline = qobject_cast<QSplineSeries *>(series))
        updateSplineSeries(spline, legendData);
#endif

    updateLegendData(series, legendData);
}

void PointRenderer::afterPolish(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_TRACE_SCOPE(QGraphs2DPointRendererAfterPolish, static_cast<int>(cleanupSeries.count()));
    for (auto series : cleanupSeries) {
        auto xySeries = qobject_cast<QXYSeries *>(series);
        if (xySeries && m_groups.contains(xySeries)) {
            auto group = m_groups.value(xySeries);

            for (auto marker : std::as_const(group->markers))
                marker->deleteLater();

            if (group->shapePath) {
                auto painterPath = group->painterPath;
                painterPath.clear();
                group->shapePath->setPath(painterPath);
            }

            delete group;
            m_groups.remove(xySeries);
        }
    }
}

void PointRenderer::updateSeries(QXYSeries *series)
{
    Q_UNUSED(series);
}

void PointRenderer::afterUpdate(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_UNUSED(cleanupSeries);
}

bool PointRenderer::handleHoverMove(QHoverEvent *event)
{
    bool handled = false;
    const QPointF &position = event->position();

    for (auto &&group : m_groups) {
        if (!group->series->isHoverable() || !group->series->isVisible())
            continue;

        auto axisRenderer = group->series->graph()->m_axisRenderer;
        auto &axisX = axisRenderer->getAxisX(group->series);
        auto &axisY = axisRenderer->getAxisY(group->series);

        bool isHNegative = axisX.maxValue < axisX.minValue;
        bool isVNegative = axisY.maxValue < axisY.minValue;

        if (group->series->type() == QAbstractSeries::SeriesType::Scatter) {
            const QString &name = group->series->name();

            bool hovering = false;

            int index = 0;
            for (auto &&rect : group->rects) {
                if (rect.contains(position.toPoint())) {
                    if (!group->hover) {
                        group->hover = true;
                        emit group->series->hoverEnter(name, position, group->series->at(index));
                    }
                    emit group->series->hover(name, position, group->series->at(index));
                    hovering = true;
                }
                index++;
            }

            if (!hovering && group->hover) {
                group->hover = false;
                emit group->series->hoverExit(name, position);
            }
        } else {
            const qreal x0 = event->position().x();
            const qreal y0 = event->position().y();

            const qreal hoverSize = defaultSize(group->series) / 2.0;
            const QString &name = group->series->name();
            auto &&points = group->series->points();
            // True when line, false when spline
            const bool isLine = group->series->type() == QAbstractSeries::SeriesType::Line;
            if (points.size() >= 2) {
                bool hovering = false;
                auto subpath = group->painterPath.toSubpathPolygons();

                if (group->painterPath.elementCount() != points.size())
                    m_graph->ensurePolished();

                for (int i = 0; i < points.size() - 1; i++) {
                    qreal x1, y1, x2, y2;
                    if (i == 0) {
                        auto element1 = group->painterPath.elementAt(0);
                        auto element2 = group->painterPath.elementAt(isLine ? 1 : 3);
                        x1 = isHNegative ? element2.x : element1.x;
                        y1 = element1.y;
                        x2 = isHNegative ? element1.x : element2.x;
                        y2 = element2.y;
                    } else {
                        bool n = isVNegative | isHNegative;
                        // Each Spline (cubicTo) has 3 elements where third one is the x & y.
                        // So content of elements are:
                        // With Spline:
                        // [0] : MoveToElement
                        // [1] : 1. CurveToElement (c1x, c1y)
                        // [2] : 1. CurveToDataElement (c2x, c2y)
                        // [3] : 1. CurveToDataElement (x, y)
                        // [4] : 2. CurveToElement (c1x, c1y)
                        // ...
                        // With Line:
                        // [0] : MoveToElement
                        // [1] : 1. LineToElement (x, y)
                        // [2] : 2. LineToElement (x, y)
                        // ...
                        int element1Index = n ? (i + 1) : i;
                        int element2Index = n ? i : (i + 1);
                        element1Index = isLine ? element1Index : element1Index * 3;
                        element2Index = isLine ? element2Index : element2Index * 3;
                        auto element1 = group->painterPath.elementAt(element1Index);
                        auto element2 = group->painterPath.elementAt(element2Index);
                        x1 = element1.x;
                        y1 = element1.y;
                        x2 = element2.x;
                        y2 = element2.y;
                    }

                    if (isLine) {
                        qreal denominator = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
                        qreal hoverDistance = qAbs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1))
                                              / qSqrt(denominator);

                        if (hoverDistance < hoverSize) {
                            qreal alpha = 0;
                            qreal extrapolation = 0;
                            if (x2 - x1 >= y2 - y1) {
                                if (x2 - x1 != 0) {
                                    alpha = ((x2 - x1) - (x0 - x1)) / qAbs(x2 - x1);
                                    extrapolation = hoverSize / qAbs(x2 - x1);
                                }
                            } else {
                                if (y2 - y1 != 0) {
                                    alpha = ((y2 - y1) - (y0 - y1)) / qAbs(y2 - y1);
                                    extrapolation = hoverSize / qAbs(y2 - y1);
                                }
                            }

                            if (alpha >= -extrapolation && alpha <= 1.0 + extrapolation) {
                                bool n = isVNegative | isHNegative;

                                const QPointF &point1 = points[n ? i + 1 : i];
                                const QPointF &point2 = points[n ? i : i + 1];

                                QPointF point = (point2 * (1.0 - alpha)) + (point1 * alpha);

                                if (!group->hover) {
                                    group->hover = true;
                                    group->series->setHovered(true);
                                    emit group->series->hoverEnter(name, position, point);
                                }

                                emit group->series->hover(name, position, point);
                                hovering = true;
                                handled = true;
                            }
                        }
                    } else { // Spline
                        auto segments = subpath[0];

                        for (auto it = segments.begin(); it != segments.end(); ++it) {
                            if (std::next(it, 1) == segments.end())
                                break;

                            auto it2 = std::next(it, 1);

                            qreal denominator = (it2->x() - it->x()) * (it2->x() - it->x())
                                                + (it2->y() - it->y()) * (it2->y() - it->y());
                            qreal hoverDistance = qAbs((it2->x() - it->x()) * (it->y() - y0)
                                                       - (it->x() - x0) * (it2->y() - it->y()))
                                                  / qSqrt(denominator);

                            if (hoverDistance < hoverSize) {
                                qreal alpha = 0;
                                qreal extrapolation = 0;
                                if (it2->x() - it->x() >= it2->y() - it->y()) {
                                    if (it2->x() - it->x() != 0) {
                                        alpha = ((it2->x() - it->x()) - (x0 - it->x()))
                                                / qAbs(it2->x() - it->x());
                                        extrapolation = hoverSize / qAbs(it2->x() - it->x());
                                    }
                                } else {
                                    if (it2->y() - it->y() != 0) {
                                        alpha = ((it2->y() - it->y()) - (y0 - it->y()))
                                                / qAbs(it2->y() - it->y());
                                        extrapolation = hoverSize / qAbs(it2->y() - it->y());
                                    }
                                }

                                if (alpha >= -extrapolation && alpha <= 1.0 + extrapolation) {
                                    qreal cx1, cy1, cx2, cy2;

                                    reverseRenderCoordinates(axisRenderer,
                                                             group->series,
                                                             it->x(),
                                                             it->y(),
                                                             &cx1,
                                                             &cy1);
                                    reverseRenderCoordinates(axisRenderer,
                                                             group->series,
                                                             it2->x(),
                                                             it2->y(),
                                                             &cx2,
                                                             &cy2);

                                    const QPointF &point1 = {cx1, cy1};
                                    const QPointF &point2 = {cx2, cy2};

                                    QPointF point = (point2 * (1.0 - alpha)) + (point1 * alpha);

                                    if (!group->hover) {
                                        group->hover = true;
                                        emit group->series->hoverEnter(name, position, point);
                                    }

                                    emit group->series->hover(name, position, point);
                                    hovering = true;
                                    handled = true;
                                }
                            }
                        }
                    }
                }

                if (!hovering && group->hover) {
                    group->hover = false;
                    group->series->setHovered(false);
                    emit group->series->hoverExit(name, position);
                    handled = true;
                }
            }
        }
    }
    return handled;
}


QPointF PointRenderer::reverseRenderCoordinates(QAbstractSeries *series, qreal x, qreal y)
{
    m_areaWidth = width();
    m_areaHeight = height();

    auto &axisX = m_graph->m_axisRenderer->getAxisX(series);
    auto &axisY = m_graph->m_axisRenderer->getAxisY(series);

    m_maxVertical = axisY.valueRange > 0 ? 1.0 / axisY.valueRange : 100.0;
    m_maxHorizontal = axisX.valueRange > 0 ? 1.0 / axisX.valueRange : 100.0;

    auto vmin = axisY.minValue > axisY.maxValue ? std::abs(axisY.minValue) : axisY.minValue;
    m_verticalOffset = (vmin / axisY.valueRange) * m_areaHeight;

    auto hmin = axisX.minValue > axisX.maxValue ? std::abs(axisX.minValue) : axisX.minValue;
    m_horizontalOffset = (hmin / axisX.valueRange) * m_areaWidth;

    qreal x0;
    qreal y0;

    reverseRenderCoordinates(m_graph->m_axisRenderer, series, x, y, &x0, &y0);

    return QPointF(x0, y0);
}

QT_END_NAMESPACE

#include "moc_pointrenderer_p.cpp"
