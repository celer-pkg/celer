// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "pierenderer_p.h"
#include <QtGraphs/qpieseries.h>
#include <QtGraphs/qpieslice.h>
#include <QtQuick/private/qquicktext_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <limits>
#include <private/qquickdraghandler_p.h>
#include <private/pierenderer_p.h>
#include <private/qabstractseries_p.h>
#include <private/qgraphsview_p.h>
#include <private/qpieseries_p.h>
#include <private/qpieslice_p.h>
#include <private/qquickshape_p.h>
#include <private/qquicksvgparser_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
               "class PieRenderer;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DPieRendererAfterPolish_entry, int cleanupSeriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DPieRendererAfterPolish_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPieRendererHandlePolish_entry, int sliceCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DPieRendererHandlePolish_exit);

constexpr qreal qrealMax = std::numeric_limits<qreal>::max();

PieRenderer::PieRenderer(QGraphsView *graph, bool clipPlotArea)
    : QQuickItem(graph)
    , m_graph(graph)
    , m_painterPath()
{
    setFlag(QQuickItem::ItemHasContents);
    setClip(clipPlotArea);

    m_shape = new QQuickShape(this);
    m_shape->setParentItem(this);
    m_shape->setPreferredRendererType(QQuickShape::CurveRenderer);

    m_tapHandler = new QQuickTapHandler(this);
    connect(m_tapHandler, &QQuickTapHandler::singleTapped, this, &PieRenderer::onSingleTapped);
    connect(m_tapHandler, &QQuickTapHandler::doubleTapped, this, &PieRenderer::onDoubleTapped);
    connect(m_tapHandler, &QQuickTapHandler::pressedChanged, this, &PieRenderer::onPressedChanged);

    m_dragHandler = new QQuickDragHandler(this);
    m_dragHandler->setTarget(nullptr);
    connect(m_dragHandler, &QQuickDragHandler::grabChanged, this, &PieRenderer::onGrabChanged);
    connect(m_dragHandler, &QQuickDragHandler::translationChanged, this,
            &PieRenderer::onTranslationChanged);
}

PieRenderer::~PieRenderer() {}

void PieRenderer::setSize(QSizeF size)
{
    QQuickItem::setSize(size);
}

void PieRenderer::updateActiveSlices(QPieSeries *series, QList<QPieSlice *> slicelist)
{
    for (QPieSlice *slice : std::as_const(slicelist)) {
        QPieSlicePrivate *d = slice->d_func();
        QQuickShapePath *shapePath = d->m_shapePath;
        QQuickShapePath *labelPath = d->m_labelPath;
        auto labelElements = labelPath->pathElements();
        auto pathElements = shapePath->pathElements();
        auto labelItem = d->m_labelItem;

        if (!m_activeSlices.contains(slice)) {
            auto data = m_shape->data();
            data.append(&data, shapePath);
            SliceData sliceData{};
            sliceData.initialized = false;
            m_activeSlices.insert(slice, sliceData);
        }

        QQuickShape *labelShape = d->m_labelShape;
        labelShape->setVisible(series->isVisible() && d->m_isLabelVisible);
        labelItem->setVisible(series->isVisible() && d->m_isLabelVisible);

        if (!series->isVisible()) {
            pathElements.clear(&pathElements);
            labelElements.clear(&labelElements);
            continue;
        }

        if (!shapePath->parent())
            shapePath->setParent(m_shape);

        if (!d->m_labelItem->parent()) {
            d->m_labelItem->setParent(this);
            d->m_labelItem->setParentItem(this);
        }

        if (!labelShape->parent()) {
            labelShape->setParent(this);
            labelShape->setParentItem(this);
        }

        updateActiveSlices(series, slice->subSlices());
    }
}

void PieRenderer::handlePolish(QPieSeries *series)
{
    auto slices = series->slices();
    updateActiveSlices(series, slices);

    if (!series->isVisible())
        return;

    QPointF center = QPointF(size().width() * series->horizontalPosition(),
                             size().height() * series->verticalPosition());
    qreal radius = size().width() > size().height() ? size().height() : size().width();
    radius *= (.5 * series->pieSize());

    QGraphsTheme *theme = m_graph->theme();

    if (!theme) {
        qCCritical(lcCritical2D, "Theme not found.");
        return;
    }

    if (m_colorIndex < 0)
        m_colorIndex = m_graph->graphSeriesCount();
    m_graph->setGraphSeriesCount(m_colorIndex + series->slices().size());

    QList<QLegendData> legendDataList;
    auto slicelist = series->slices();
    Q_TRACE(QGraphs2DPieRendererHandlePolish_entry, static_cast<int>(slicelist.count()));
    handleSlicesPolish(series, legendDataList, slicelist, series->startAngle(), series->endAngle(),
                       center, radius, 1.0);
    Q_TRACE(QGraphs2DPieRendererHandlePolish_exit);

    series->d_func()->setLegendData(legendDataList);
}

void PieRenderer::handleSlicesPolish(QPieSeries *series,
                                     QList<QLegendData> &legendDataList,
                                     QList<QPieSlice *> slicelist,
                                     qreal startAngle,
                                     qreal endAngle,
                                     QPointF center,
                                     qreal radius,
                                     qreal radiusRatio)
{
    QGraphsTheme *theme = m_graph->theme();

    int sliceIndex = 0;
    qreal sliceAngle = startAngle;
    for (QPieSlice *slice : std::as_const(slicelist)) {
        m_painterPath.clear();

        QPieSlicePrivate *d = slice->d_func();
        d->setStartAngle(sliceAngle);
        d->setAngleSpan((endAngle - startAngle) * slice->percentage()
                        * series->valuesMultiplier());

        // update slice
        QQuickShapePath *shapePath = d->m_shapePath;

        // border color
        const auto &borderColors = theme->borderColors();
        int index = sliceIndex % borderColors.size();
        QColor borderColor = borderColors.at(index);
        if (d->m_borderColor.isValid() && d->m_borderColor.alpha() != 0)
            borderColor = d->m_borderColor;

        // border width
        qreal borderWidth = theme->borderWidth();
        if (d->m_borderWidth >= 1.0)
            borderWidth = d->m_borderWidth;

        // color
        const auto &seriesColors = theme->seriesColors();
        index = sliceIndex % seriesColors.size();
        QColor color = seriesColors.at(index);
        if (d->m_color.isValid() && d->m_color.alpha() != 0)
            color = d->m_color;

        shapePath->setStrokeWidth(borderWidth);
        shapePath->setStrokeColor(borderColor);
        shapePath->setFillColor(color);

        QColor labelTextColor = theme->labelTextColor();
        if (d->m_labelColor.isValid())
            labelTextColor = d->m_labelColor;
        d->m_labelItem->setColor(labelTextColor);
        d->m_labelPath->setStrokeColor(labelTextColor);

        if (!m_activeSlices.contains(slice))
            return;

        qreal radian = qDegreesToRadians(slice->startAngle());
        qreal startBigX = radius * qSin(radian) * radiusRatio;
        qreal startBigY = radius * qCos(radian) * radiusRatio;

        qreal startSmallX = startBigX * series->holeSize();
        qreal startSmallY = startBigY * series->holeSize();

        qreal explodeDistance = .0;
        if (slice->isExploded())
            explodeDistance = slice->explodeDistanceFactor() * radius;
        radian = qDegreesToRadians(slice->startAngle() + (slice->angleSpan() * .5));
        qreal xShift = center.x() + (explodeDistance * qSin(radian));
        qreal yShift = center.y() - (explodeDistance * qCos(radian));

        qreal pointX = startBigY * qSin(radian) + startBigX * qCos(radian);
        qreal pointY = startBigY * qCos(radian) - startBigX * qSin(radian);

        qreal newCenterX = center.x() + (explodeDistance * qSin(radian));
        qreal newCenterY = center.y() - (explodeDistance * qCos(radian));
        QRectF rect(newCenterX - radius * radiusRatio,
                    newCenterY - radius * radiusRatio,
                    radius * 2 * radiusRatio,
                    radius * 2 * radiusRatio);

        shapePath->setStartX(center.x());
        shapePath->setStartY(center.y());

        if (series->holeSize() > 0) {
            QRectF insideRect(center.x() - series->holeSize() * radius
                                  + (explodeDistance * qSin(radian)),
                              center.y() - series->holeSize() * radius
                                  - (explodeDistance * qCos(radian)),
                              series->holeSize() * radius * 2,
                              series->holeSize() * radius * 2);

            m_painterPath.arcMoveTo(rect, -slice->startAngle() + 90);
            m_painterPath.arcTo(rect, -slice->startAngle() + 90, -slice->angleSpan());
            m_painterPath.arcTo(insideRect,
                                -slice->startAngle() + 90 - slice->angleSpan(),
                                slice->angleSpan());
            m_painterPath.closeSubpath();
        } else {
            m_painterPath.moveTo(rect.center());
            m_painterPath.arcTo(rect, -slice->startAngle() + 90, -slice->angleSpan());
            m_painterPath.closeSubpath();
        }

        radian = qDegreesToRadians(slice->angleSpan());

        pointX = startSmallY * qSin(radian) + startSmallX * qCos(radian);
        pointY = startSmallY * qCos(radian) - startSmallX * qSin(radian);

        d->m_largeArc = {xShift + pointX, yShift - pointY};

        shapePath->setPath(m_painterPath);
        m_painterPath.clear();

        radian = qDegreesToRadians(slice->startAngle() + (slice->angleSpan() * .5));
        startBigX = radius * qSin(radian) * radiusRatio;
        startBigY = radius * qCos(radian) * radiusRatio;

        pointX = radius * (1.0 + d->m_labelArmLengthFactor) * qSin(radian);
        pointY = radius * (1.0 + d->m_labelArmLengthFactor) * qCos(radian);

        m_painterPath.moveTo(xShift + startBigX, yShift - startBigY);
        m_painterPath.lineTo(xShift + pointX, yShift - pointY);

        d->m_centerLine = {xShift + pointX, yShift - pointY};

        d->m_labelArm = {xShift + pointX, yShift - pointY};

        auto labelWidth = radian > M_PI ? -d->m_labelItem->width() : d->m_labelItem->width();
        m_painterPath.lineTo(d->m_labelArm.x() + labelWidth, d->m_labelArm.y());

        d->setLabelPosition(d->m_labelPosition);
        d->m_labelPath->setPath(m_painterPath);

        sliceAngle += slice->angleSpan();
        sliceIndex++;

        handleSlicesPolish(series,
            legendDataList,
            slice->subSlices(),
            slice->startAngle(),
            slice->startAngle() + slice->angleSpan(),
            {newCenterX, newCenterY},
            radius,
            radiusRatio * slice->subSlicesRatio());

        legendDataList.push_back({color, borderColor, d->m_labelText});
    }
}

void PieRenderer::afterPolish(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_TRACE(QGraphs2DPieRendererAfterPolish_entry, static_cast<int>(cleanupSeries.count()));
    for (auto series : cleanupSeries) {
        auto pieSeries = qobject_cast<QPieSeries *>(series);
        if (pieSeries)
            handleSlicesAfterPolish(pieSeries->slices());
    }
}

void PieRenderer::handleSlicesAfterPolish(QList<QPieSlice *> slicelist)
{
    for (QPieSlice *slice : std::as_const(slicelist)) {
        QPieSlicePrivate *d = slice->d_func();
        auto labelElements = d->m_labelPath->pathElements();
        auto shapeElements = d->m_shapePath->pathElements();

        labelElements.clear(&labelElements);
        shapeElements.clear(&shapeElements);

        slice->deleteLater();
        d->m_labelItem->deleteLater();

        m_activeSlices.remove(slice);

        handleSlicesAfterPolish(slice->subSlices());
    }
    Q_TRACE(QGraphs2DPieRendererAfterPolish_exit);
}

void PieRenderer::updateSeries(QPieSeries *series)
{
    auto needPolish = false;

    for (auto &sliceData : m_activeSlices) {
        if (!sliceData.initialized) {
            sliceData.initialized = true;
            needPolish = true;
        }
    }

    if (needPolish)
        handlePolish(series);
}

void PieRenderer::afterUpdate(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_UNUSED(cleanupSeries);
}

void PieRenderer::markedDeleted(QList<QPieSlice *> deleted)
{
    auto emptyPath = QPainterPath{};

    for (auto slice : deleted) {
        auto d = slice->d_func();
        d->m_shapePath->setPath(emptyPath);
        d->m_labelPath->setPath(emptyPath);
        d->m_labelItem->deleteLater();
        m_activeSlices.remove(slice);
    }
    // We could mark m_currentHoverSlice null only if
    // it matches to a deleted slice, but as removals
    // affect other slices positions it is probably
    // better to just always disable current hovering.
    m_currentHoverSlice = nullptr;
}

bool PieRenderer::isPointInSlice(QPointF point, QPieSlice *slice, qreal *angle)
{
    QPieSeries* series = slice->series();
    QPointF center = QPointF(size().width() * series->horizontalPosition(),
                             size().height() * series->verticalPosition());
    qreal radius = size().width() > size().height() ? size().height() : size().width();
    radius *= (.5 * series->pieSize());

    qreal explodeDistance = .0;
    if (slice->isExploded())
        explodeDistance = slice->explodeDistanceFactor() * radius;
    qreal radian = qDegreesToRadians(slice->startAngle() + (slice->angleSpan() * .5));
    qreal xShift = center.x() + explodeDistance * qSin(radian);
    qreal yShift = center.y() - explodeDistance * qCos(radian);

    QPointF adjustedPosition = QPointF(point.x() - xShift,
                                       point.y() - yShift);
    qreal foundAngle = qRadiansToDegrees(qAtan2(adjustedPosition.y(), adjustedPosition.x())) + 90;
    if (foundAngle < 0)
        foundAngle += 360;

    if (angle)
        *angle = foundAngle;

    // Check if we are in a sub slice
    if (isPointInSubSlices(point, slice))
        return false;

    QPieSlicePrivate *d = slice->d_func();
    QQuickShapePath *shapePath = d->m_shapePath;
    QPainterPath painterPath = shapePath->path();
    return painterPath.contains(point);
}

bool PieRenderer::isPointInSubSlices(QPointF point, QPieSlice *slice)
{
    auto slices = slice->subSlices();
    for (const auto &subSlice : std::as_const(slices)) {
        QPieSlicePrivate *d = subSlice->d_func();
        QQuickShapePath *shapePath = d->m_shapePath;
        QPainterPath painterPath = shapePath->path();
        if (painterPath.contains(point))
            return true;
        if (isPointInSubSlices(point, subSlice))
            return true;
    }
    return false;
}

qreal PieRenderer::distanceToSegment(const QVector2D p, const QVector2D segmentStart,
                                     const QVector2D segmentEnd)
{
    static qreal maxdistance = 15.0;
    QVector2D line(segmentEnd - segmentStart);
    QVector2D startToP(p - segmentStart);
    qreal t = QVector2D::dotProduct(startToP, line) / line.lengthSquared();

    t = qBound(0.0, t, 1.0);
    QVector2D proj = segmentStart + t * line;
    qreal distance = (p - proj).length();
    return distance <= maxdistance ? distance : qrealMax;
}

bool PieRenderer::handleHoverMove(QHoverEvent *event)
{
    bool handled = false;
    const QPointF &position = event->position();

    bool hovering = false;
    QList<QPieSlice *> list = m_activeSlices.keys();
    for (const auto &slice : std::as_const(list)) {
        if (!slice->series()->isHoverable())
            continue;

        qreal angle;
        if (isPointInSlice(position, slice, &angle)) {
            const QString &name = slice->series()->name();
            const QPointF value(slice->startAngle(), angle);

            if (!m_currentHoverSlice) {
                m_currentHoverSlice = slice;
                slice->series()->setHovered(true);
                emit slice->series()->hoverEnter(name, position, value);
            }
            if (m_currentHoverSlice != slice) {
                slice->series()->setHovered(true);
                emit m_currentHoverSlice->series()->hoverExit(name, position);
                emit slice->series()->hoverEnter(name, position, value);
                m_currentHoverSlice = slice;
            }

            emit slice->series()->hover(name, position, value);
            hovering = true;
            handled = true;
        }
    }

    if (!hovering && m_currentHoverSlice) {
        m_currentHoverSlice->series()->setHovered(false);
        emit m_currentHoverSlice->series()->
            hoverExit(m_currentHoverSlice->series()->name(), position);
        m_currentHoverSlice = nullptr;
        handled = true;
    }
    return handled;
}

void PieRenderer::onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    QList<QPieSlice *> list = m_activeSlices.keys();
    for (const auto &pieSlice : std::as_const(list)) {
        if (!pieSlice->series()->isSelectable())
            continue;

        if (isPointInSlice(eventPoint.position(), pieSlice)) {
            emit pieSlice->series()->clicked(pieSlice);
            return;
        }
    }
}

void PieRenderer::onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    QList<QPieSlice *> list = m_activeSlices.keys();
    for (const auto &pieSlice : std::as_const(list)) {
        if (!pieSlice->series()->isSelectable())
            continue;

        if (isPointInSlice(eventPoint.position(), pieSlice)) {
            emit pieSlice->series()->doubleClicked(pieSlice);
            return;
        }
    }
}

void PieRenderer::onPressedChanged()
{
    QList<QPieSlice *> list = m_activeSlices.keys();
    for (const auto &pieSlice : std::as_const(list)) {
        if (!pieSlice->series()->isSelectable())
            continue;

        if (isPointInSlice(m_tapHandler->point().position(), pieSlice)) {
            if (m_tapHandler->isPressed())
                emit pieSlice->series()->pressed(pieSlice);
            else
                emit pieSlice->series()->released(pieSlice);
            return;
        }
    }
}

void PieRenderer::onGrabChanged(QPointingDevice::GrabTransition transition, QEventPoint eventPoint)
{
    QList<QPieSlice *> slices = m_activeSlices.keys();
    if (transition == QPointingDevice::GrabTransition::GrabPassive) {
        const qreal radiusRatio = 1.0;
        for (const auto &slice : std::as_const(slices)) {
            if (isPointInSlice(eventPoint.position(), slice)) {
                m_dragSlice = slice;
                // Slice side length
                qreal radius = size().width() > size().height() ? size().height() : size().width();
                radius *= (.5 * slice->series()->pieSize());
                m_dragState.dragSeriesRadius = radius;
                qreal startAngle = -slice->startAngle() + 90;
                qreal startRadian = qDegreesToRadians(startAngle);
                qreal endRadian = qDegreesToRadians(startAngle - slice->angleSpan());
                QVector2D center(size().width() * slice->series()->horizontalPosition(),
                                 size().height() * slice->series()->verticalPosition());
                m_dragState.dragSeriesCenter = center;
                QVector2D leftEndPoint(center.x() + (radius * radiusRatio * qCos(startRadian)),
                                       center.y() - (radius * radiusRatio * qSin(startRadian)));
                QVector2D rightEndPoint(center.x() + (radius * radiusRatio * qCos(endRadian)),
                                        center.y() - (radius * radiusRatio * qSin(endRadian)));

                qreal d_left =
                        distanceToSegment(QVector2D(eventPoint.position()), center, leftEndPoint);
                qreal d_right =
                        distanceToSegment(QVector2D(eventPoint.position()), center, rightEndPoint);

                if (d_left < qrealMax && d_left < d_right)
                    m_closerEdge = Qt::LeftEdge;
                else if (d_right < qrealMax && d_left > d_right)
                    m_closerEdge = Qt::RightEdge;
                else
                    return;

                m_dragState.dragging = true;
                break;
            }
        }
    } else if (transition == QPointingDevice::UngrabPassive) {
        m_dragSlice = nullptr;
        m_dragState.dragging = false;
    }
}

void PieRenderer::onTranslationChanged(QVector2D delta)
{
    if (!m_dragState.dragging)
        return;

    qreal startAngle = -m_dragSlice->startAngle() + 90;
    qreal startRadian = qDegreesToRadians(startAngle);
    qreal endRadian = qDegreesToRadians(startAngle - m_dragSlice->angleSpan());
    QVector2D leftEndPoint(
            m_dragState.dragSeriesCenter.x() + (m_dragState.dragSeriesRadius * qCos(startRadian)),
            m_dragState.dragSeriesCenter.y() - (m_dragState.dragSeriesRadius * qSin(startRadian)));
    QVector2D rightEndPoint(
            m_dragState.dragSeriesCenter.x() + (m_dragState.dragSeriesRadius * qCos(endRadian)),
            m_dragState.dragSeriesCenter.y() - (m_dragState.dragSeriesRadius * qSin(endRadian)));
    // Calculate the edge
    QVector2D edgeCenter;
    if (m_closerEdge == Qt::LeftEdge)
        edgeCenter = (leftEndPoint - m_dragState.dragSeriesCenter) / 2;
    else if (m_closerEdge == Qt::RightEdge)
        edgeCenter = (rightEndPoint - m_dragState.dragSeriesCenter) / 2;

    QVector2D perpendicular(-edgeCenter.y(), edgeCenter.x());
    perpendicular.normalize();
    delta.normalize();
    float dragAmount = QVector2D::dotProduct(delta, perpendicular);
    if (qAbs(dragAmount) < qCos((qDegreesToRadians(45))))
        return;

    const float sensitivity = 0.1f;
    qreal change = dragAmount * sensitivity;
    change = m_closerEdge == Qt::RightEdge ? change : change * -1;
    m_dragSlice->setValue(m_dragSlice->value() + change);
}

QT_END_NAMESPACE

#include "moc_pierenderer_p.cpp"
