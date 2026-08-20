// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifdef USE_BARGRAPH
#include <QtGraphs/QBarCategoryAxis>
#endif
#include <QtGraphs/QGraphsTheme>
#include <private/axisrenderer_p.h>
#include <private/qabstractaxis_p.h>
#include <private/qbarseries_p.h>
#include <private/qdatetimeaxis_p.h>
#include <private/qgraphsview_p.h>
#include <private/qvalueaxis_p.h>
#include <QtQuick/private/qquickdraghandler_p.h>
#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
              "#include <qnamespace.h>" \
              "class AxisRenderer;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_METADATA(qtgraphs, "ENUM { } Qt::Orientation;")

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateDateTimeXAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateDateTimeXAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateAxisLabelItems_entry, int labelItemCount, int neededCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateAxisLabelItems_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateDateTimeYAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateDateTimeYAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateValueXAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateValueXAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateValueYAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateValueYAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateBarYAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateBarYAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateBarXAxisLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateBarXAxisLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateAxis_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererUpdateAxis_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererHandlePolish_entry, Qt::Orientation orientation);
Q_TRACE_POINT(qtgraphs, QGraphs2DAxisRendererHandlePolish_exit);

AxisRenderer::AxisRenderer(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_graph = qobject_cast<QGraphsView *>(parent);
    setFlag(QQuickItem::ItemHasContents);
}

AxisRenderer::~AxisRenderer() {}

QGraphsTheme *AxisRenderer::theme() {
    return m_graph->m_theme;
}

void AxisRenderer::initialize() {
    if (m_initialized) {
        return;
    }
    if (!window()) {
        qCCritical(lcCritical2D, "window doesn't exist.");
        return;
    }

    if (m_axisGrid)
        m_axisGrid->componentComplete();
    if (m_axisGridShadow)
        m_axisGridShadow->componentComplete();

    for (auto&& ax : *m_horzAxes) {
        if (ax.line)
            ax.line->componentComplete();
        if (ax.ticker)
            ax.ticker->componentComplete();
        if (ax.lineShadow)
            ax.lineShadow->componentComplete();
        if (ax.tickerShadow)
            ax.tickerShadow->componentComplete();
    }

    for (auto&& ax : *m_vertAxes) {
        if (ax.line)
            ax.line->componentComplete();
        if (ax.ticker)
            ax.ticker->componentComplete();
        if (ax.lineShadow)
            ax.lineShadow->componentComplete();
        if (ax.tickerShadow)
            ax.tickerShadow->componentComplete();
    }

    m_initialized = true;
}

QVector2D AxisRenderer::windowToAxisCoords(QVector2D coords)
{
    float x = coords.x();
    float y = coords.y();
    x /= width() - m_graph->m_marginLeft - m_graph->m_marginRight - m_graph->m_axisWidth;
    y /= height() - m_graph->m_marginTop - m_graph->m_marginBottom - m_graph->m_axisHeight;
    x *= (*m_horzAxes)[0].valueRange;
    y *= (*m_vertAxes)[0].valueRange;
    return QVector2D(x, y);
}

bool AxisRenderer::calculateZoom(QAbstractAxis *axis, qreal delta)
{
    if (axis->type() == QAbstractAxis::AxisType::Value) {
        auto valueAxis = qobject_cast<QValueAxis *>(axis);
        qreal zoom = 1.0;
        zoom = valueAxis->zoom();

        qreal change = 0.0;
        if (delta > 0)
            change = zoom * m_graph->m_zoomSensitivity;
        else if (delta < 0)
            change = -zoom * m_graph->m_zoomSensitivity;

        zoom += change;

        if (zoom < 0.01f)
            zoom = 0.01;

        valueAxis->setZoom(zoom);
        return true;
    } else if (axis->type() == QAbstractAxis::AxisType::DateTime) {
        auto dateAxis = qobject_cast<QDateTimeAxis *>(axis);
        qreal zoom = 1.0;
        zoom = dateAxis->zoom();

        qreal change = 0.0;
        if (delta > 0)
            change = zoom * m_graph->m_zoomSensitivity;
        else if (delta < 0)
            change = -zoom * m_graph->m_zoomSensitivity;

        zoom += change;

        if (zoom < 0.01f)
            zoom = 0.01;

        dateAxis->setZoom(zoom);
        return true;
    }
    return false;
}

bool AxisRenderer::zoom(qreal delta)
{
    if (m_graph->zoomStyle() != QGraphsView::ZoomStyle::Center)
        return false;

    bool hzoomed = calculateZoom((*m_horzAxes)[0].axis, delta);
    bool vzoomed = calculateZoom((*m_vertAxes)[0].axis, delta);
    return hzoomed && vzoomed;
}

const AxisRenderer::AxisProperties &AxisRenderer::getAxisX(QAbstractSeries *series) const
{
    for (auto &&ax : *m_horzAxes) {
        if (ax.axis && (ax.axis == series->axisX() || ax.axis == series->axisY()))
            return ax;
    }
    return (*m_horzAxes)[0];
}

const AxisRenderer::AxisProperties &AxisRenderer::getAxisY(QAbstractSeries *series) const
{
    for (auto &&ax : *m_vertAxes) {
        if (ax.axis && (ax.axis == series->axisX() || ax.axis == series->axisY()))
            return ax;
    }
    return (*m_vertAxes)[0];
}

bool AxisRenderer::handleWheel(QWheelEvent *event)
{
    return zoom(-event->angleDelta().y());
}

void AxisRenderer::handlePinchScale(qreal delta)
{
    zoom(delta - 1.0);
}

void AxisRenderer::handlePinchGrab(QPointingDevice::GrabTransition transition, QEventPoint point)
{
    Q_UNUSED(transition)
    Q_UNUSED(point)
}

void AxisRenderer::onTranslationChanged(QVector2D delta)
{
    if (!m_dragState.dragging)
        return;

    m_dragState.delta += delta;

    auto hax = (*m_horzAxes)[0].axis;
    auto vax = (*m_vertAxes)[0].axis;

    if (m_graph->zoomAreaEnabled() && m_graph->m_zoomAreaItem) {
        m_graph->m_zoomAreaItem->setVisible(true);

        qreal x = m_dragState.touchPositionAtPress.x();
        if (m_dragState.delta.x() < 0)
            x += m_dragState.delta.x();

        qreal y = m_dragState.touchPositionAtPress.y();
        if (m_dragState.delta.y() < 0)
            y += m_dragState.delta.y();

        qreal width = qAbs(m_dragState.delta.x());
        qreal height = qAbs(m_dragState.delta.y());

        m_graph->m_zoomAreaItem->setX(x);
        m_graph->m_zoomAreaItem->setY(y);
        m_graph->m_zoomAreaItem->setWidth(width);
        m_graph->m_zoomAreaItem->setHeight(height);
    }

    if (m_graph->panStyle() != QGraphsView::PanStyle::Drag)
        return;

    QVector2D change(m_dragState.delta);
    change = windowToAxisCoords(change);
    change.setX(-change.x());

    if (hax->type() == QAbstractAxis::AxisType::Value) {
        auto axis = qobject_cast<QValueAxis *>(hax);
        axis->setPan(m_dragState.panAtPress.x() + change.x());
    } else if (hax->type() == QAbstractAxis::AxisType::DateTime) {
        auto axis = qobject_cast<QDateTimeAxis *>(hax);
        axis->setPan(m_dragState.panAtPress.x() + change.x());
    }

    if (vax->type() == QAbstractAxis::AxisType::Value) {
        auto axis = qobject_cast<QValueAxis *>(vax);
        axis->setPan(m_dragState.panAtPress.y() + change.y());
    } else if (vax->type() == QAbstractAxis::AxisType::DateTime) {
        auto axis = qobject_cast<QDateTimeAxis *>(vax);
        axis->setPan(m_dragState.panAtPress.y() + change.y());
    }
}

void AxisRenderer::onGrabChanged(QPointingDevice::GrabTransition transition, QEventPoint point)
{
    const QPointF position = point.position();

    auto &hax = (*m_horzAxes)[0];
    auto &vax = (*m_vertAxes)[0];
    auto htype = hax.axis->type();
    auto vtype = vax.axis->type();

    if (transition == QPointingDevice::GrabPassive
        && point.pressPosition() == point.position()) {

        m_dragState.dragging = true;
        m_dragState.touchPositionAtPress = QVector2D(position);
        m_dragState.delta = QVector2D(0, 0);

        if (htype == QAbstractAxis::AxisType::Value) {
            auto axis = qobject_cast<QValueAxis *>(hax.axis);
            m_dragState.panAtPress.setX(axis->pan());
        } else if (htype == QAbstractAxis::AxisType::DateTime) {
            auto axis = qobject_cast<QDateTimeAxis *>(hax.axis);
            m_dragState.panAtPress.setX(axis->pan());
        }

        if (vtype == QAbstractAxis::AxisType::Value) {
            auto axis = qobject_cast<QValueAxis *>(vax.axis);
            m_dragState.panAtPress.setY(axis->pan());
        } else if (vtype == QAbstractAxis::AxisType::DateTime) {
            auto axis = qobject_cast<QDateTimeAxis *>(vax.axis);
            m_dragState.panAtPress.setY(axis->pan());
        }
    } else if (m_dragState.dragging && transition == QPointingDevice::UngrabExclusive) {
        m_dragState.dragging = false;

        if (!m_graph->zoomAreaEnabled())
            return;

        if (m_graph->m_zoomAreaItem)
            m_graph->m_zoomAreaItem->setVisible(false);

        QVector2D zoomBoxEnd(position);
        auto center = (m_dragState.touchPositionAtPress + zoomBoxEnd) / 2;
        auto size = (m_dragState.touchPositionAtPress - zoomBoxEnd);
        size.setX(qAbs(size.x()));
        size.setY(qAbs(size.y()));

        if (int(size.x()) == 0 || int(size.y()) == 0)
            return;

        size = windowToAxisCoords(size);

        center -= QVector2D(m_graph->m_marginLeft + m_graph->m_axisWidth, m_graph->m_marginTop);
        center = windowToAxisCoords(center);
        center -= QVector2D(hax.valueRange / 2.0f, vax.valueRange / 2.0f);

        if (htype == QAbstractAxis::AxisType::Value) {
            auto axis = qobject_cast<QValueAxis *>(hax.axis);
            axis->setZoom(hax.valueRangeZoomless / size.x());
            axis->setPan(axis->pan() + center.x());
        } else {
            auto axis = qobject_cast<QDateTimeAxis *>(hax.axis);
            axis->setZoom(hax.valueRangeZoomless / size.x());
            axis->setPan(axis->pan() + center.x());
        }

        if (vtype == QAbstractAxis::AxisType::Value) {
            auto axis = qobject_cast<QValueAxis *>(vax.axis);
            axis->setZoom(vax.valueRangeZoomless / size.y());
            axis->setPan(axis->pan() - center.y());
        } else {
            auto axis = qobject_cast<QDateTimeAxis *>(vax.axis);
            axis->setZoom(vax.valueRangeZoomless / size.y());
            axis->setPan(axis->pan() - center.y());
        }
    }
}

void AxisRenderer::handlePolish()
{
    if (m_graph->panStyle() != QGraphsView::PanStyle::None
        || m_graph->zoomStyle() != QGraphsView::ZoomStyle::None || m_graph->zoomAreaEnabled()) {
        if (!m_dragHandler)
            createDragHandler();
    } else if (m_dragHandler) {
        deleteDragHandler();
    }

    // See if series is horizontal, so axis should also switch places.
    bool vertical = true;
    if (m_graph->orientation() == Qt::Orientation::Horizontal)
        vertical = false;

    Q_TRACE(QGraphs2DAxisRendererHandlePolish_entry, m_graph->orientation());
    if (vertical) {
        m_horzAxes = &m_axes1;
        m_vertAxes = &m_axes2;
    } else {
        m_horzAxes = &m_axes2;
        m_vertAxes = &m_axes1;
    }

    if (vertical != m_wasVertical) {
        // Orientation has changed, so clear possible custom elements
        for (auto&& ax : m_axes1) {
            if (ax.title)
                ax.title->deleteLater();
            if (ax.line)
                ax.line->deleteLater();
            if (ax.ticker)
                ax.ticker->deleteLater();
            if (ax.lineShadow)
                ax.lineShadow->deleteLater();
            if (ax.tickerShadow)
                ax.tickerShadow->deleteLater();
            for (auto&& item : ax.textItems)
                item->deleteLater();
        }

        for (auto&& ax : m_axes2) {
            if (ax.title)
                ax.title->deleteLater();
            if (ax.line)
                ax.line->deleteLater();
            if (ax.ticker)
                ax.ticker->deleteLater();
            if (ax.lineShadow)
                ax.lineShadow->deleteLater();
            if (ax.tickerShadow)
                ax.tickerShadow->deleteLater();
            for (auto&& item : ax.textItems)
                item->deleteLater();
        }

        m_axes1.clear();
        m_axes2.clear();

        m_wasVertical = vertical;
    }

    if (m_axes1.empty())
        m_axes1.emplace_back();

    if (m_axes2.empty())
        m_axes2.emplace_back();

    m_axes1[0].axis = m_graph->axisX();
    m_axes2[0].axis = m_graph->axisY();

    for (auto&& s : m_graph->m_seriesList) {
        if (auto series = qobject_cast<QAbstractSeries *>(s)) {
            if (series->axisX() && series->axisX() != m_graph->axisX()) {
                bool contains = false;
                for (auto&& ax : m_axes1) {
                    if (ax.axis == series->axisX()) {
                        contains = true;
                        break;
                    }
                }

                if (!contains) {
                    auto &ax = m_axes1.emplace_back();
                    ax.axis = series->axisX();
                }
            }

            if (series->axisY() && series->axisY() != m_graph->axisY()) {
                bool contains = false;
                for (auto&& ax : m_axes2) {
                    if (ax.axis == series->axisY()) {
                        contains = true;
                        break;
                    }
                }

                if (!contains) {
                    auto &ax = m_axes2.emplace_back();
                    ax.axis = series->axisY();
                }
            }
        }
    }

    if (!m_axisGrid) {
        m_axisGrid = new AxisGrid(this);
        m_axisGrid->setZ(-1);
        m_axisGrid->setupShaders();
        m_axisGrid->setOrigo(0);
    }
    if (!m_axisGridShadow) {
        m_axisGridShadow = new AxisGrid(this);
        m_axisGridShadow->setZ(-3);
        m_axisGridShadow->setupShaders();
        m_axisGridShadow->setOrigo(0);
    }

    for (int i = 1; i < m_axes1.size(); i++) {
        auto& ax = m_axes1[i];

        bool used = false;

        for (auto&& s : m_graph->m_seriesList) {
            if (auto series = qobject_cast<QAbstractSeries *>(s)) {
                if (series->axisX() && series->axisX() == ax.axis) {
                    used = true;
                    break;
                }
            }
        }

        if (!used) {
            if (ax.title)
                ax.title->deleteLater();
            if (ax.line)
                ax.line->deleteLater();
            if (ax.ticker)
                ax.ticker->deleteLater();
            if (ax.lineShadow)
                ax.lineShadow->deleteLater();
            if (ax.tickerShadow)
                ax.tickerShadow->deleteLater();
            for (auto&& item : ax.textItems)
                item->deleteLater();
            m_axes1.removeAt(i);
            i--;
            continue;
        }
    }

    for (int i = 1; i < m_axes2.size(); i++) {
        auto& ax = m_axes2[i];

        bool used = false;

        for (auto&& s : m_graph->m_seriesList) {
            if (auto series = qobject_cast<QAbstractSeries *>(s)) {
                if (series->axisY() && series->axisY() == ax.axis) {
                    used = true;
                    break;
                }
            }
        }

        if (!used) {
            if (ax.title)
                ax.title->deleteLater();
            if (ax.line)
                ax.line->deleteLater();
            if (ax.ticker)
                ax.ticker->deleteLater();
            if (ax.lineShadow)
                ax.lineShadow->deleteLater();
            if (ax.tickerShadow)
                ax.tickerShadow->deleteLater();
            for (auto&& item : ax.textItems)
                item->deleteLater();
            m_axes2.removeAt(i);
            i--;
            continue;
        }
    }

    for (int i = 0; i < m_vertAxes->size(); i++) {
        auto& ax = (*m_vertAxes)[i];

        if (!ax.line) {
            ax.line = new AxisLine(this);
            ax.line->setZ(-1);
            ax.line->setupShaders();
        }
        if (!ax.ticker) {
            ax.ticker = new AxisTicker(this);
            ax.ticker->setZ(-2);
            ax.ticker->setOrigo(0);
            // TODO: Configurable in theme or axis?
            ax.ticker->setSubTickLength(0.5);
            ax.ticker->setupShaders();
        }
        if (!ax.lineShadow) {
            ax.lineShadow = new AxisLine(this);
            ax.lineShadow->setZ(-3);
            ax.lineShadow->setupShaders();
        }
        if (!ax.tickerShadow) {
            ax.tickerShadow = new AxisTicker(this);
            ax.tickerShadow->setZ(-3);
            ax.tickerShadow->setOrigo(0);
            // TODO: Configurable in theme or axis?
            ax.tickerShadow->setSubTickLength(ax.ticker->subTickLength());
            ax.tickerShadow->setupShaders();
        }
    }

    for (int i = 0; i < m_horzAxes->size(); i++) {
        auto& ax = (*m_horzAxes)[i];

        if (!ax.line) {
            ax.line = new AxisLine(this);
            ax.line->setZ(-1);
            ax.line->setIsHorizontal(true);
            ax.line->setupShaders();
        }
        if (!ax.ticker) {
            ax.ticker = new AxisTicker(this);
            ax.ticker->setZ(-2);
            ax.ticker->setIsHorizontal(true);
            ax.ticker->setOrigo(0);
            // TODO: Configurable in theme or axis?
            ax.ticker->setSubTickLength(0.2);
            ax.ticker->setupShaders();
        }
        if (!ax.lineShadow) {
            ax.lineShadow = new AxisLine(this);
            ax.lineShadow->setZ(-3);
            ax.lineShadow->setupShaders();
        }
        if (!ax.tickerShadow) {
            ax.tickerShadow = new AxisTicker(this);
            ax.tickerShadow->setZ(-3);
            ax.tickerShadow->setIsHorizontal(true);
            ax.tickerShadow->setOrigo(0);
            // TODO: Configurable in theme or axis?
            ax.tickerShadow->setSubTickLength(ax.ticker->subTickLength());
            ax.tickerShadow->setupShaders();
        }
    }
    Q_TRACE(QGraphs2DAxisRendererHandlePolish_exit);

    updateAxis();
}

void AxisRenderer::updateAxis()
{
    if (!theme())
        return;

    float axisWidth = m_graph->m_axisWidth;
    float axisHeight = m_graph->m_axisHeight;

    const bool gridVisible = theme()->isGridVisible();
    if ((*m_vertAxes)[0].axis) {
        m_gridVerticalLinesVisible = gridVisible && (*m_vertAxes)[0].axis->isGridVisible();
        m_gridVerticalSubLinesVisible = gridVisible && (*m_vertAxes)[0].axis->isSubGridVisible();
    }
    if ((*m_horzAxes)[0].axis) {
        m_gridHorizontalLinesVisible = gridVisible && (*m_horzAxes)[0].axis->isGridVisible();
        m_gridHorizontalSubLinesVisible = gridVisible && (*m_horzAxes)[0].axis->isSubGridVisible();
    }

    int topCount = 0;
    int leftCount = 0;
    int xCount = 0;
    int yCount = 0;

    int topTitleCount = 0;
    int leftTitleCount = 0;
    int xTitleCount = 0;
    int yTitleCount = 0;

    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateAxis);
    for (auto &&ax : *m_horzAxes) {
        if (ax.axis && (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)) {
            topCount++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                topTitleCount++;
        }
        if (ax.axis) {
            xCount++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                xTitleCount++;
        }
    }

    for (auto&& ax : *m_vertAxes) {
        if (ax.axis && (ax.axis->alignment() == Qt::AlignLeft || ax.axis->alignment() == Qt::AlignTop)) {
            leftCount++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                leftTitleCount++;
        }
        if (ax.axis) {
            yCount++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                yTitleCount++;
        }
    }

    if (xTitleCount > 0) {
        xTitleCount--;
        if (xTitleCount > 0)
            xTitleCount--;
    }

    if (yTitleCount > 0) {
        yTitleCount--;
        if (yTitleCount > 0)
            yTitleCount--;
    }

    if (leftTitleCount > 0)
        leftTitleCount--;

    if (topTitleCount > 0)
        topTitleCount--;

    int top = 0;
    int bottom = 0;
    int topTitle = 0;
    int bottomTitle = 0;
    for (auto&& ax : *m_horzAxes) {
        if (!ax.axis)
            continue;

        ax.x = leftCount * m_graph->m_axisWidth + leftTitleCount * m_graph->m_axisTitleMargin;

        if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft) {
            ax.y = (topCount - top - 1) * m_graph->m_axisHeight
                   + (topTitleCount - topTitle) * m_graph->m_axisTitleMargin;
            top++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                topTitle++;
        } else if (ax.axis->alignment() == Qt::AlignBottom || ax.axis->alignment() == Qt::AlignRight) {
            ax.y = bottom * m_graph->m_axisHeight + bottomTitle * m_graph->m_axisTitleMargin;
            bottom++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                bottomTitle++;
        }
    }

    int left = 0;
    int right = 0;
    int leftTitle = 0;
    int rightTitle = 0;
    for (auto&& ax : *m_vertAxes) {
        if (!ax.axis)
            continue;

        ax.y = topCount * m_graph->m_axisHeight + topTitleCount * m_graph->m_axisTitleMargin;

        if (ax.axis->alignment() == Qt::AlignLeft || ax.axis->alignment() == Qt::AlignTop) {
            ax.x = (leftCount - left - 1) * m_graph->m_axisWidth
                   + (leftTitleCount - leftTitle) * m_graph->m_axisTitleMargin;
            left++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                leftTitle++;
        } else if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom) {
            ax.x = right * m_graph->m_axisWidth + rightTitle * m_graph->m_axisTitleMargin;
            right++;
            if (ax.axis->isTitleVisible() && !ax.axis->titleText().isEmpty())
                rightTitle++;
        }
    }

    for (auto&& ax : *m_vertAxes) {
        if (auto vaxis = qobject_cast<QValueAxis *>(ax.axis)) {
            double step = vaxis->tickInterval();

            qreal diff = vaxis->max() - vaxis->min();
            qreal center = diff / 2.0f + vaxis->min() + vaxis->pan();

            diff /= vaxis->zoom();

            ax.maxValue = center + diff / 2.0f;
            ax.minValue = center - diff / 2.0f;

            ax.valueRange = ax.maxValue - ax.minValue;
            ax.valueRangeZoomless = vaxis->max() - vaxis->min();

            // If step is not manually defined (or it is invalid), calculate autostep
            if (step <= 0)
                step = getValueStepsFromRange(vaxis->max() - vaxis->min());

            // Get smallest tick label value
            double minLabel = vaxis->tickAnchor();
            while (minLabel < ax.minValue)
                minLabel += step;
            while (minLabel >= (ax.minValue + step))
                minLabel -= step;
            ax.minLabel = minLabel;

            ax.valueStep = step;
            int axisVerticalSubTickCount = vaxis->subTickCount();
            ax.subGridScale = axisVerticalSubTickCount > 0 ? 1.0 / (axisVerticalSubTickCount + 1)
                                                            : 1.0;
            ax.stepPx = (height() - m_graph->m_marginTop - m_graph->m_marginBottom
                         - axisHeight * xCount - m_graph->m_axisTitleMargin * xTitleCount)
                        / (ax.valueRange / ax.valueStep);
            double axisVerticalValueDiff = ax.minLabel - ax.minValue;
            ax.displacement = -(axisVerticalValueDiff / ax.valueStep) * ax.stepPx;

            // Update value labels
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom)
                updateValueYAxisLabels(ax, m_graph->m_y2AxisLabelsArea);
            else
                updateValueYAxisLabels(ax, m_graph->m_y1AxisLabelsArea);
        }
    }
    for (auto&& ax : *m_horzAxes) {
        if (auto haxis = qobject_cast<QValueAxis *>(ax.axis)) {
            double step = haxis->tickInterval();

            qreal diff = haxis->max() - haxis->min();
            qreal center = diff / 2.0f + haxis->min() + haxis->pan();

            diff /= haxis->zoom();

            ax.maxValue = center + diff / 2.0f;
            ax.minValue = center - diff / 2.0f;

            ax.valueRange = ax.maxValue - ax.minValue;
            ax.valueRangeZoomless = haxis->max() - haxis->min();

            // If step is not manually defined (or it is invalid), calculate autostep
            if (step <= 0)
                step = getValueStepsFromRange(haxis->max() - haxis->min());

            // Get smallest tick label value
            double minLabel = haxis->tickAnchor();
            while (minLabel < ax.minValue)
                minLabel += step;
            while (minLabel >= (ax.minValue + step))
                minLabel -= step;
            ax.minLabel = minLabel;

            ax.valueStep = step;
            int axisHorizontalSubTickCount = haxis->subTickCount();
            ax.subGridScale = axisHorizontalSubTickCount > 0 ?
                    1.0 / (axisHorizontalSubTickCount + 1) : 1.0;
            ax.stepPx = (width() - m_graph->m_marginLeft - m_graph->m_marginRight
                         - axisWidth * yCount - m_graph->m_axisTitleMargin * yTitleCount)
                        / (ax.valueRange / ax.valueStep);
            double axisHorizontalValueDiff = ax.minLabel - ax.minValue;
            ax.displacement = -(axisHorizontalValueDiff / ax.valueStep) * ax.stepPx;

            // Update value labels
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)
                updateValueXAxisLabels(ax, m_graph->m_x2AxisLabelsArea);
            else
                updateValueXAxisLabels(ax, m_graph->m_x1AxisLabelsArea);
        }
    }

#ifdef USE_BARGRAPH
    for (auto&& ax : *m_vertAxes) {
        if (auto vaxis = qobject_cast<QBarCategoryAxis *>(ax.axis)) {
            ax.maxValue = vaxis->categories().size();
            ax.minValue = 0;
            ax.valueRange = ax.maxValue - ax.minValue;
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom)
                updateBarYAxisLabels(ax, m_graph->m_y2AxisLabelsArea);
            else
                updateBarYAxisLabels(ax, m_graph->m_y1AxisLabelsArea);
        }
    }

    for (auto&& ax : *m_horzAxes) {
        if (auto haxis = qobject_cast<QBarCategoryAxis *>(ax.axis)) {
            ax.maxValue = haxis->categories().size();
            ax.minValue = 0;
            ax.valueRange = ax.maxValue - ax.minValue;
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)
                updateBarXAxisLabels(ax, m_graph->m_x2AxisLabelsArea);
            else
                updateBarXAxisLabels(ax, m_graph->m_x1AxisLabelsArea);
        }
    }
#endif

    for (auto&& ax : *m_vertAxes) {
        if (auto vaxis = qobject_cast<QDateTimeAxis *>(ax.axis)) {
            // Todo: make constant for all axis, or clamp in class? (QTBUG-124736)
            const double MAX_DIVS = 100.0;

            double interval = std::clamp<double>(vaxis->tickInterval(), 0.0, MAX_DIVS);
            qreal diff = vaxis->max().toMSecsSinceEpoch() - vaxis->min().toMSecsSinceEpoch();
            qreal center = diff / 2.0f + vaxis->min().toMSecsSinceEpoch()
                                       + vaxis->pan();

            diff /= vaxis->zoom();
            ax.maxValue = center + diff / 2.0f;
            ax.minValue = center - diff / 2.0f;
            ax.valueRange = std::abs(ax.maxValue - ax.minValue);
            ax.valueRangeZoomless = vaxis->max().toMSecsSinceEpoch()
                                  - vaxis->min().toMSecsSinceEpoch();

            // in ms
            double segment;
            if (interval <= 0) {
                segment = getValueStepsFromRange(ax.valueRange);
                interval = ax.valueRange / segment;
            } else {
                segment = ax.valueRange / interval;
            }

            ax.minLabel = std::clamp(interval, 1.0, MAX_DIVS);

            ax.valueStep = segment;
            int axisVerticalSubTickCount = vaxis->subTickCount();
            ax.subGridScale = axisVerticalSubTickCount > 0
                                               ? 1.0 / (axisVerticalSubTickCount + 1)
                                               : 1.0;
            ax.stepPx = (height() - m_graph->m_marginTop - m_graph->m_marginBottom
                                    - axisHeight)
                                   / (qFuzzyIsNull(segment)
                                          ? interval
                                          : (ax.valueRange / ax.valueStep));
            double axisVerticalValueDiff = fmod(ax.minValue, ax.valueStep) / ax.valueStep;
            ax.displacement = axisVerticalValueDiff * ax.stepPx;

            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom)
                updateDateTimeYAxisLabels(ax, m_graph->m_y2AxisLabelsArea);
            else
                updateDateTimeYAxisLabels(ax, m_graph->m_y1AxisLabelsArea);
        }
    }

    for (auto&& ax : *m_horzAxes) {
        if (auto haxis = qobject_cast<QDateTimeAxis *>(ax.axis)) {
            const double MAX_DIVS = 100.0;

            double interval = std::clamp<double>(haxis->tickInterval(), 0.0, MAX_DIVS);
            double diff = haxis->max().toMSecsSinceEpoch() - haxis->min().toMSecsSinceEpoch();
            double center = diff / 2.0f + haxis->min().toMSecsSinceEpoch()
                                        + haxis->pan();

            diff /= haxis->zoom();
            ax.maxValue = center + diff / 2.0f;
            ax.minValue = center - diff / 2.0f;
            ax.valueRange = std::abs(ax.maxValue - ax.minValue);
            ax.valueRangeZoomless = haxis->max().toMSecsSinceEpoch()
                                  - haxis->min().toMSecsSinceEpoch();

            // in ms
            double segment;
            if (interval <= 0) {
                segment = getValueStepsFromRange(ax.valueRange);
                interval = ax.valueRange / segment;
            } else {
                segment = ax.valueRange / interval;
            }

            ax.minLabel = std::clamp(interval, 1.0, MAX_DIVS);

            ax.valueStep = segment;
            int axisHorizontalSubTickCount = haxis->subTickCount();
            ax.subGridScale = axisHorizontalSubTickCount > 0
                                                 ? 1.0 / (axisHorizontalSubTickCount + 1)
                                                 : 1.0;
            ax.stepPx = (width() - m_graph->m_marginLeft - m_graph->m_marginRight
                                      - axisWidth)
                                     / (qFuzzyIsNull(segment)
                                            ? interval
                                            : (ax.valueRange / ax.valueStep));

            double axisHorizontalValueDiff = fmod(ax.minValue, ax.valueStep) / ax.valueStep;
            ax.displacement = axisHorizontalValueDiff * ax.stepPx;

            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)
                updateDateTimeXAxisLabels(ax, m_graph->m_x2AxisLabelsArea);
            else
                updateDateTimeXAxisLabels(ax, m_graph->m_x1AxisLabelsArea);
        }
    }

    updateAxisTickers();
    updateAxisTickersShadow();
    updateAxisGrid();
    updateAxisGridShadow();
    updateAxisTitles();
}

void AxisRenderer::updateAxisTickers()
{
    for (auto&& ax : *m_vertAxes) {
        if (ax.axis) {
            QRectF yAxisRect;
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom)
                yAxisRect = m_graph->m_y2AxisTickersArea;
            else
                yAxisRect = m_graph->m_y1AxisTickersArea;

            // Note: Fix before enabling, see QTBUG-121207 and QTBUG-121211
            //if (theme()->themeDirty()) {
            if (ax.axis->subColor().isValid())
                ax.ticker->setSubTickColor(ax.axis->subColor());
            else
                ax.ticker->setSubTickColor(theme()->axisY().subColor());
            if (ax.axis->color().isValid())
                ax.ticker->setTickColor(ax.axis->color());
            else
                ax.ticker->setTickColor(theme()->axisY().mainColor());
            ax.ticker->setTickLineWidth(theme()->axisY().mainWidth());
            ax.ticker->setSubTickLineWidth(theme()->axisY().subWidth());
            ax.ticker->setSmoothing(m_graph->axisYSmoothing());
            //}

                float topPadding = m_axisGrid->gridLineWidth() * 0.5;
                float bottomPadding = topPadding;
                // TODO Only when changed
                ax.ticker->setDisplacement(ax.displacement);
                QRectF &rect = yAxisRect;
                ax.ticker->setX(rect.x() + ax.x);
                ax.ticker->setY(rect.y() + ax.y);
                ax.ticker->setWidth(rect.width());
                ax.ticker->setHeight(rect.height());
                ax.ticker->setFlipped(ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom);

                ax.ticker->setSpacing((ax.ticker->height() - topPadding - bottomPadding)
                                       / (ax.valueRange / ax.valueStep));
                ax.ticker->setSubTicksVisible(!qFuzzyCompare(ax.subGridScale, 1.0));
                ax.ticker->setSubTickScale(ax.subGridScale);
                ax.ticker->setVisible(ax.axis->isVisible());
                // Axis line
                if (ax.axis->color().isValid())
                    ax.line->setColor(ax.axis->color());
                else
                    ax.line->setColor(theme()->axisY().mainColor());
                ax.line->setLineWidth(theme()->axisY().mainWidth());
                ax.line->setSmoothing(m_graph->axisYSmoothing());

                float xMovement = 0.5 * (ax.line->lineWidth() + ax.line->smoothing());
                if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom)
                    ax.line->setX(ax.ticker->x() - xMovement);
                else
                    ax.line->setX(ax.ticker->x() + ax.ticker->width() - xMovement);
                ax.line->setY(ax.ticker->y());
                ax.line->setWidth(ax.line->lineWidth() + ax.line->smoothing());
                ax.line->setHeight(ax.ticker->height());
                ax.line->setVisible(ax.axis->isLineVisible());
        } else {
            // Hide all parts of vertical axis
            ax.ticker->setVisible(false);
            ax.line->setVisible(false);
            for (auto &textItem : ax.textItems)
                textItem->setVisible(false);
        }
    }

    for (auto&& ax : *m_horzAxes) {
        if (ax.axis) {
            QRectF xAxisRect;

            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)
                xAxisRect = m_graph->m_x2AxisTickersArea;
            else
                xAxisRect = m_graph->m_x1AxisTickersArea;

            //if (theme()->themeDirty()) {
            if (ax.axis->subColor().isValid())
                ax.ticker->setSubTickColor(ax.axis->subColor());
            else
                ax.ticker->setSubTickColor(theme()->axisX().subColor());
            if (ax.axis->color().isValid())
                ax.ticker->setTickColor(ax.axis->color());
            else
                ax.ticker->setTickColor(theme()->axisX().mainColor());
            ax.ticker->setTickLineWidth(theme()->axisX().mainWidth());
            ax.ticker->setSubTickLineWidth(theme()->axisX().subWidth());
            ax.ticker->setSmoothing(m_graph->axisXSmoothing());
            //}

            float leftPadding = m_axisGrid->gridLineWidth() * 0.5;
            float rightPadding = leftPadding;
            // TODO Only when changed
            ax.ticker->setDisplacement(ax.displacement);
            QRectF &rect = xAxisRect;
            ax.ticker->setX(rect.x() + ax.x);
            ax.ticker->setY(rect.y() + ax.y);
            ax.ticker->setWidth(rect.width());
            ax.ticker->setHeight(rect.height());
            ax.ticker->setFlipped(ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft);

            ax.ticker->setSpacing((ax.ticker->width() - leftPadding - rightPadding)
                                             / (ax.valueRange / ax.valueStep));
            ax.ticker->setSubTicksVisible(!qFuzzyCompare(ax.subGridScale, 1.0));
            ax.ticker->setSubTickScale(ax.subGridScale);
            ax.ticker->setVisible(ax.axis->isVisible());
            // Axis line
            if (ax.axis->color().isValid())
                ax.line->setColor(ax.axis->color());
            else
                ax.line->setColor(theme()->axisX().mainColor());
            ax.line->setLineWidth(theme()->axisX().mainWidth());
            ax.line->setSmoothing(m_graph->axisXSmoothing());
            ax.line->setX(ax.ticker->x());
            float yMovement = 0.5 * (ax.line->lineWidth() + ax.line->smoothing());
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft)
                ax.line->setY(ax.ticker->y() + ax.ticker->height() - yMovement);
            else
                ax.line->setY(ax.ticker->y() - yMovement);
            ax.line->setWidth(ax.ticker->width());
            ax.line->setHeight(ax.line->lineWidth() + ax.line->smoothing());
            ax.line->setVisible(ax.axis->isLineVisible());
        } else {
            // Hide all parts of horizontal axis
            ax.ticker->setVisible(false);
            ax.line->setVisible(false);
            for (auto &textItem : ax.textItems)
                textItem->setVisible(false);
        }
    }
}

void AxisRenderer::updateAxisTickersShadow()
{
    for (auto&& ax : *m_vertAxes) {
        if (ax.axis && m_graph->isShadowVisible()) {
            ax.tickerShadow->setSubTickColor(m_graph->shadowColor());
            ax.tickerShadow->setTickColor(m_graph->shadowColor());
            ax.tickerShadow->setSubTickLineWidth(ax.ticker->subTickLineWidth()
                                                  + m_graph->shadowBarWidth());
            ax.tickerShadow->setTickLineWidth(ax.ticker->tickLineWidth()
                                               + m_graph->shadowBarWidth());
            ax.tickerShadow->setSmoothing(ax.ticker->smoothing() + m_graph->shadowSmoothing());

            // TODO Only when changed
            ax.tickerShadow->setDisplacement(ax.ticker->displacement());
            ax.tickerShadow->setX(ax.ticker->x() + m_graph->shadowXOffset());
            ax.tickerShadow->setY(ax.ticker->y() + m_graph->shadowYOffset()
                                   + m_graph->shadowBarWidth() * 0.5);
            ax.tickerShadow->setWidth(ax.ticker->width());
            ax.tickerShadow->setHeight(ax.ticker->height());
            ax.tickerShadow->setFlipped(ax.ticker->isFlipped());
            ax.tickerShadow->setSpacing(ax.ticker->spacing());
            ax.tickerShadow->setSubTicksVisible(ax.ticker->subTicksVisible());
            ax.tickerShadow->setSubTickScale(ax.ticker->subTickScale());
            ax.tickerShadow->setVisible(ax.ticker->isVisible());
            // Axis line
            ax.lineShadow->setColor(m_graph->shadowColor());
            ax.lineShadow->setLineWidth(ax.line->lineWidth() + m_graph->shadowBarWidth());
            ax.lineShadow->setSmoothing(ax.line->smoothing() + m_graph->shadowSmoothing());
            ax.lineShadow->setX(ax.line->x() + m_graph->shadowXOffset());
            ax.lineShadow->setY(ax.line->y() + m_graph->shadowYOffset()
                                 + m_graph->shadowBarWidth() * 0.5);
            ax.lineShadow->setWidth(ax.line->width());
            ax.lineShadow->setHeight(ax.line->height());
            ax.lineShadow->setVisible(ax.line->isVisible());
        } else {
            // Hide all parts of vertical axis
            ax.tickerShadow->setVisible(false);
            ax.lineShadow->setVisible(false);
        }
    }

    for (auto&& ax : *m_horzAxes) {
        if (ax.axis && m_graph->isShadowVisible()) {
            ax.tickerShadow->setSubTickColor(m_graph->shadowColor());
            ax.tickerShadow->setTickColor(m_graph->shadowColor());
            ax.tickerShadow->setSubTickLineWidth(ax.ticker->subTickLineWidth()
                                                  + m_graph->shadowBarWidth());
            ax.tickerShadow->setTickLineWidth(ax.ticker->tickLineWidth()
                                               + m_graph->shadowBarWidth());
            ax.tickerShadow->setSmoothing(ax.ticker->smoothing() + m_graph->shadowSmoothing());

            // TODO Only when changed
            ax.tickerShadow->setDisplacement(ax.ticker->displacement());
            ax.tickerShadow->setX(ax.ticker->x() + m_graph->shadowXOffset()
                                   - m_graph->shadowBarWidth() * 0.5);
            ax.tickerShadow->setY(ax.ticker->y() + m_graph->shadowYOffset());
            ax.tickerShadow->setWidth(ax.ticker->width());
            ax.tickerShadow->setHeight(ax.ticker->height());
            ax.tickerShadow->setFlipped(ax.ticker->isFlipped());
            ax.tickerShadow->setSpacing(ax.ticker->spacing());
            ax.tickerShadow->setSubTicksVisible(ax.ticker->subTicksVisible());
            ax.tickerShadow->setSubTickScale(ax.ticker->subTickScale());
            ax.tickerShadow->setVisible(ax.ticker->isVisible());
            // Axis line
            ax.lineShadow->setColor(m_graph->shadowColor());
            ax.lineShadow->setLineWidth(ax.line->width() + m_graph->shadowBarWidth());
            ax.lineShadow->setSmoothing(ax.line->smoothing() + m_graph->shadowSmoothing());
            ax.lineShadow->setX(ax.line->x() + m_graph->shadowXOffset()
                                 - m_graph->shadowBarWidth() * 0.5);
            ax.lineShadow->setY(ax.line->y() + m_graph->shadowYOffset());
            ax.lineShadow->setWidth(ax.line->width());
            ax.lineShadow->setHeight(ax.line->height());
            ax.lineShadow->setVisible(ax.line->isVisible());
        } else {
            // Hide all parts of horizontal axis
            ax.tickerShadow->setVisible(false);
            ax.lineShadow->setVisible(false);
        }
    }
}

void AxisRenderer::updateAxisGrid()
{
    auto &hax = (*m_horzAxes)[0];
    auto &vax = (*m_vertAxes)[0];

    m_axisGrid->setGridColor(theme()->grid().mainColor());
    m_axisGrid->setSubGridColor(theme()->grid().subColor());
    m_axisGrid->setSubGridLineWidth(theme()->grid().subWidth());
    m_axisGrid->setGridLineWidth(theme()->grid().mainWidth());
    const double minimumSmoothing = 0.05;
    m_axisGrid->setSmoothing(m_graph->gridSmoothing() + minimumSmoothing);
    if (theme()->isPlotAreaBackgroundVisible())
        m_axisGrid->setPlotAreaBackgroundColor(theme()->plotAreaBackgroundColor());
    else
        m_axisGrid->setPlotAreaBackgroundColor(QColorConstants::Transparent);

    float topPadding = m_axisGrid->gridLineWidth() * 0.5;
    float bottomPadding = topPadding;
    float leftPadding = topPadding;
    float rightPadding = topPadding;
    // TODO Only when changed
    m_axisGrid->setGridMovement(QPointF(hax.displacement, vax.displacement));
    QRectF rect = m_graph->m_plotArea;
    m_axisGrid->setX(rect.x());
    m_axisGrid->setY(rect.y());
    m_axisGrid->setWidth(rect.width());
    m_axisGrid->setHeight(rect.height());

    m_axisGrid->setGridWidth((m_axisGrid->width() - leftPadding - rightPadding)
                             / (hax.valueRange / hax.valueStep));
    m_axisGrid->setGridHeight((m_axisGrid->height() - topPadding - bottomPadding)
                              / (vax.valueRange / vax.valueStep));
    m_axisGrid->setGridVisibility(QVector4D(m_gridHorizontalLinesVisible,
                                            m_gridVerticalLinesVisible,
                                            m_gridHorizontalSubLinesVisible,
                                            m_gridVerticalSubLinesVisible));
    m_axisGrid->setVerticalSubGridScale(vax.subGridScale);
    m_axisGrid->setHorizontalSubGridScale(hax.subGridScale);
}

void AxisRenderer::updateAxisGridShadow()
{
    if (m_graph->isShadowVisible()) {
        m_axisGridShadow->setGridColor(m_graph->shadowColor());
        m_axisGridShadow->setSubGridColor(m_graph->shadowColor());
        m_axisGridShadow->setSubGridLineWidth(m_axisGrid->subGridLineWidth() + m_graph->shadowBarWidth());
        m_axisGridShadow->setGridLineWidth(m_axisGrid->gridLineWidth() + m_graph->shadowBarWidth());
        m_axisGridShadow->setSmoothing(m_axisGrid->smoothing() + m_graph->shadowSmoothing());

        // TODO Only when changed
        m_axisGridShadow->setGridMovement(m_axisGrid->gridMovement());
        m_axisGridShadow->setX(m_axisGrid->x() + m_graph->shadowXOffset() - m_graph->shadowBarWidth() * 0.5);
        m_axisGridShadow->setY(m_axisGrid->y() + m_graph->shadowYOffset() + m_graph->shadowBarWidth() * 0.5);
        m_axisGridShadow->setWidth(m_axisGrid->width());
        m_axisGridShadow->setHeight(m_axisGrid->height());
        m_axisGridShadow->setGridWidth(m_axisGrid->gridWidth());
        m_axisGridShadow->setGridHeight(m_axisGrid->gridHeight());
        m_axisGridShadow->setGridVisibility(m_axisGrid->gridVisibility());
        m_axisGridShadow->setVerticalSubGridScale(m_axisGrid->verticalSubGridScale());
        m_axisGridShadow->setHorizontalSubGridScale(m_axisGrid->horizontalSubGridScale());
        m_axisGridShadow->setVisible(true);
    } else {
        m_axisGridShadow->setVisible(false);
    }
}

void AxisRenderer::updateAxisTitles()
{
    QRectF xAxisRect;
    QRectF yAxisRect;

    for (auto &&ax : *m_horzAxes) {
        if (!ax.title)
            ax.title = new QQuickText(this);

        if (ax.axis && ax.axis->isTitleVisible()) {
            ax.title->setVAlign(QQuickText::AlignVCenter);
            ax.title->setHAlign(QQuickText::AlignHCenter);
            ax.title->setText(ax.axis->titleText());
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft) {
                xAxisRect = m_graph->m_x2AxisLabelsArea;
                ax.title->setY(xAxisRect.y() - ax.title->height() * 0.5 + ax.y);
            } else {
                xAxisRect = m_graph->m_x1AxisLabelsArea;
                ax.title->setY(xAxisRect.y() + xAxisRect.height() + ax.y);
            }

            ax.title->setX((2 * xAxisRect.x() - ax.title->width() + xAxisRect.width()) * 0.5 + ax.x);
            if (ax.axis->titleColor().isValid())
                ax.title->setColor(ax.axis->titleColor());
            else
                ax.title->setColor(theme()->labelTextColor());
            ax.title->setFont(ax.axis->titleFont());
            ax.title->setVisible(true);
        } else {
            ax.title->setVisible(false);
        }
    }

    for (auto &&ax : *m_vertAxes) {
        if (!ax.title)
            ax.title = new QQuickText(this);

        if (ax.axis && ax.axis->isTitleVisible()) {
            ax.title->setVAlign(QQuickText::AlignVCenter);
            ax.title->setHAlign(QQuickText::AlignHCenter);
            ax.title->setText(ax.axis->titleText());
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom) {
                yAxisRect = m_graph->m_y2AxisLabelsArea;
                ax.title->setX(yAxisRect.x() + ax.title->height() * 1.5 - ax.title->width() * 0.5
                               + ax.x + m_graph->m_axisTitleMargin * 0.5);
            } else {
                yAxisRect = m_graph->m_y1AxisLabelsArea;
                ax.title->setX(yAxisRect.x() + ax.title->height() - ax.title->width() * 0.5 + ax.x
                               - m_graph->m_axisTitleMargin * 0.5);
            }

            ax.title->setY((2 * yAxisRect.y() - ax.title->height() + yAxisRect.height()) * 0.5
                           + ax.y);
            ax.title->setRotation(-90);
            if (ax.axis->titleColor().isValid())
                ax.title->setColor(ax.axis->titleColor());
            else
                ax.title->setColor(theme()->labelTextColor());
            ax.title->setFont(ax.axis->titleFont());
            ax.title->setVisible(true);
        } else {
            ax.title->setVisible(false);
        }
    }
}

void AxisRenderer::updateAxisLabelItems(QList<QQuickItem *> &textItems,
                                        qsizetype neededSize, QQmlComponent *component)
{
    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateAxisLabelItems, textItems.count(),
                  static_cast<int>(neededSize));
    qsizetype currentTextItemsSize = textItems.size();
    if (currentTextItemsSize < neededSize) {
        for (qsizetype i = currentTextItemsSize; i <= neededSize; i++) {
            QQuickItem *item = nullptr;
            if (component) {
                item = qobject_cast<QQuickItem *>(
                    component->create(component->creationContext()));
            }
            if (!item)
                item = new QQuickText();
            item->setParent(this);
            item->setParentItem(this);
            textItems << item;
        }
    } else if (neededSize < currentTextItemsSize) {
        // Hide unused text items
        for (qsizetype i = neededSize;  i < currentTextItemsSize; i++) {
            auto textItem = textItems[i];
            textItem->setVisible(false);
        }
    }
}

void AxisRenderer::setLabelTextProperties(QQuickItem *item, const QString &text, bool xAxis,
                                          QQuickText::HAlignment hAlign, QQuickText::VAlignment vAlign,
                                          Qt::TextElideMode elide)
{
    if (auto textItem = qobject_cast<QQuickText *>(item)) {
        // If the component is a Text item (default), then text
        // properties can be set directly.
        textItem->setText(text);
        textItem->setHeight(textItem->contentHeight()); // Default height
        textItem->setHAlign(hAlign);
        textItem->setVAlign(vAlign);
        if (xAxis) {
            textItem->setFont(theme()->axisXLabelFont());
            textItem->setColor(theme()->axisX().labelTextColor());
        } else {
            textItem->setFont(theme()->axisYLabelFont());
            textItem->setColor(theme()->axisY().labelTextColor());
        }

        QQuickText::TextElideMode e;
        switch (elide) {
        case Qt::ElideLeft:
            e = QQuickText::ElideLeft;
            break;
        case Qt::ElideRight:
            e = QQuickText::ElideRight;
            break;
        case Qt::ElideMiddle:
            e = QQuickText::ElideMiddle;
            break;
        default:
            e = QQuickText::ElideNone;
        }
        textItem->setElideMode(e);

    } else {
        // Check for specific dynamic properties
        if (item->property("text").isValid())
            item->setProperty("text", text);
    }
}

#ifdef USE_BARGRAPH
void AxisRenderer::updateBarXAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QBarCategoryAxis *>(ax.axis);
    if (!axis)
        return;

    qsizetype categoriesCount = axis->categories().size();
    // See if we need more text items
    updateAxisLabelItems(ax.textItems, categoriesCount, axis->labelDelegate());

    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateBarXAxisLabels);
    int textIndex = 0;
    auto categories = axis->categories();
    for (const auto &category : std::as_const(categories)) {
        auto &textItem = ax.textItems[textIndex];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posX = rect.x() + ((float)textIndex / categoriesCount) *  rect.width() + ax.x;
            if (axis->labelPosition() == QBarCategoryAxis::LabelPosition::OnValue)
                posX += (1.0 / categoriesCount * rect.width() * 0.5);
            textItem->setX(posX);
            float posY = rect.y() + ax.y;
            textItem->setY(posY);
            textItem->setWidth(rect.width() / categoriesCount);
            textItem->setRotation(axis->labelsAngle());
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft) {
                setLabelTextProperties(textItem, category, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignBottom,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, category, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignTop,
                                       axis->textElideMode());
            }
            textItem->setHeight(rect.height());
            textItem->setVisible(true);
            theme()->dirtyBits()->axisXDirty = false;
        } else {
            textItem->setVisible(false);
        }
        textIndex++;
    }
}

void AxisRenderer::updateBarYAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QBarCategoryAxis *>(ax.axis);
    if (!axis)
        return;

    qsizetype categoriesCount = axis->categories().size();
    // See if we need more text items
    updateAxisLabelItems(ax.textItems, categoriesCount, axis->labelDelegate());

    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateBarYAxisLabels);
    int textIndex = 0;
    auto categories = axis->categories();
    for (const auto &category : std::as_const(categories)) {
        auto &textItem = ax.textItems[textIndex];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posX = rect.x() + ax.x;
            textItem->setX(posX);
            float posY = rect.y() + ((float)textIndex / categoriesCount) *  rect.height() + ax.y;
            if (axis->labelPosition() == QBarCategoryAxis::LabelPosition::OnValue)
                posY -= (1.0 / categoriesCount * rect.height() * 0.5);
            textItem->setY(posY);
            textItem->setWidth(rect.width());
            textItem->setRotation(axis->labelsAngle());
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom) {
                setLabelTextProperties(textItem, category, false,
                                       QQuickText::HAlignment::AlignLeft,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, category, false,
                                       QQuickText::HAlignment::AlignRight,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            }
            textItem->setHeight(rect.height() / categoriesCount);
            textItem->setVisible(true);
            theme()->dirtyBits()->axisYDirty = false;
        } else {
            textItem->setVisible(false);
        }
        textIndex++;
    }
}
#endif

void AxisRenderer::updateValueYAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QValueAxis *>(ax.axis);
    if (!axis)
        return;

    // Create label values in the range
    QList<double> yAxisLabelValues;
    const int MAX_LABELS_COUNT = 100;
    if (m_graph->orientation() == Qt::Vertical) {
        for (double i = ax.minLabel; i <= ax.maxValue; i += ax.valueStep) {
            yAxisLabelValues << i;
            if (yAxisLabelValues.size() >= MAX_LABELS_COUNT)
                break;
        }
    } else {
        for (double i = ax.maxValue; i >= ax.minLabel; i -= ax.valueStep) {
            yAxisLabelValues << i;
            if (yAxisLabelValues.size() >= MAX_LABELS_COUNT)
                break;
        }
    }
    qsizetype categoriesCount = yAxisLabelValues.size();

    // See if we need more text items
    updateAxisLabelItems(ax.textItems, categoriesCount, axis->labelDelegate());

    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateValueYAxisLabels);
    for (int i = 0;  i < categoriesCount; i++) {
        auto &textItem = ax.textItems[i];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posX = rect.x() + ax.x;
            textItem->setX(posX);
            float posY = rect.y() + rect.height() - (((float)i) * ax.stepPx) + ax.displacement;
            const double titleMargin = 0.01;
            if ((posY - titleMargin) > (rect.height() + rect.y()) || (posY + titleMargin) < rect.y()) {
                // Hide text item which are outside the axis area
                textItem->setVisible(false);
                continue;
            }
            posY += ax.y;
            textItem->setY(posY);
            textItem->setWidth(rect.width());
            textItem->setRotation(axis->labelsAngle());
            double number = yAxisLabelValues.at(i);
            // Format the number
            int decimals = axis->labelDecimals();
            if (decimals < 0)
                decimals = getValueDecimalsFromRange(ax.valueRange);
            const QString f = axis->labelFormat();
            QString label;
            if (f.length() <= 1) {
              char format = f.isEmpty() ? 'f' : f.front().toLatin1();
              label = QString::number(number, format, decimals);
            } else {
              QByteArray array = f.toLatin1();
              label = QString::asprintf(array.constData(), number);
            }
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom) {
                setLabelTextProperties(textItem, label, false,
                                       QQuickText::HAlignment::AlignLeft,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, label, false,
                                       QQuickText::HAlignment::AlignRight,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            }
            textItem->setHeight(0);
            textItem->setVisible(true);
            theme()->dirtyBits()->axisYDirty = false;
        } else {
            textItem->setVisible(false);
        }
    }
}

void AxisRenderer::updateValueXAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QValueAxis *>(ax.axis);
    if (!axis)
        return;

    // Create label values in the range
    QList<double> axisLabelValues;
    const int MAX_LABELS_COUNT = 100;
    for (double i = ax.minLabel; i <= ax.maxValue; i += ax.valueStep) {
        axisLabelValues << i;
        if (axisLabelValues.size() >= MAX_LABELS_COUNT)
            break;
    }
    qsizetype categoriesCount = axisLabelValues.size();

    // See if we need more text items
    updateAxisLabelItems(ax.textItems, categoriesCount, axis->labelDelegate());
    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateValueXAxisLabels);

    for (int i = 0;  i < categoriesCount; i++) {
        auto &textItem = ax.textItems[i];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posY = rect.y() + ax.y;
            textItem->setY(posY);
            float textItemWidth = 20;
            float posX = rect.x() + (((float)i) * ax.stepPx) - ax.displacement;
            const double titleMargin = 0.01;
            if ((posX - titleMargin) > (rect.width() + rect.x()) || (posX + titleMargin) < rect.x()) {
                // Hide text item which are outside the axis area
                textItem->setVisible(false);
                continue;
            }
            // Take text size into account only after hiding
            posX -= 0.5 * textItemWidth;
            posX += ax.x;
            textItem->setX(posX);
            textItem->setWidth(textItemWidth);
            textItem->setRotation(axis->labelsAngle());
            double number = axisLabelValues.at(i);
            // Format the number
            int decimals = axis->labelDecimals();
            if (decimals < 0)
                decimals = getValueDecimalsFromRange(ax.valueRange);
            const QString f = axis->labelFormat();
            QString label;
            if (f.length() <= 1) {
              char format = f.isEmpty() ? 'f' : f.front().toLatin1();
              label = QString::number(number, format, decimals);
            } else {
              QByteArray array = f.toLatin1();
              label = QString::asprintf(array.constData(), number);
            }
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft) {
                setLabelTextProperties(textItem, label, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignBottom,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, label, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignTop,
                                       axis->textElideMode());
            }
            textItem->setHeight(rect.height());
            textItem->setVisible(true);
            theme()->dirtyBits()->axisXDirty = false;
        } else {
            textItem->setVisible(false);
        }
    }
}

void AxisRenderer::updateDateTimeYAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QDateTimeAxis *>(ax.axis);
    if (!axis)
        return;

    qint64 maxDate = ax.maxValue;
    qint64 minDate = ax.minValue;
    int dateTimeSize = ax.minLabel + 1;
    qint64 segment = (maxDate - minDate)
                   / ax.minLabel;

    qint64 anchor = (minDate / segment) * segment;

    // See if we need more text items
    updateAxisLabelItems(ax.textItems, dateTimeSize, axis->labelDelegate());

    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateDateTimeYAxisLabels);
    for (auto i = 0; i < dateTimeSize; ++i) {
        auto &textItem = ax.textItems[i];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posX = rect.x() + ax.x;
            textItem->setX(posX);
            float posY = rect.y() + rect.height() - (((float) i) * ax.stepPx) + ax.displacement;
            const double titleMargin = 0.01;
            if ((posY - titleMargin) > (rect.height() + rect.y())
                || (posY + titleMargin) < rect.y()) {
                // Hide text item which are outside the axis area
                textItem->setVisible(false);
                continue;
            }
            posY += ax.y;
            textItem->setY(posY);
            textItem->setWidth(rect.width());
            textItem->setRotation(axis->labelsAngle());
            auto date = QDateTime::fromMSecsSinceEpoch(anchor + (segment * i));
            QString label = date.toString(axis->labelFormat());
            if (ax.axis->alignment() == Qt::AlignRight || ax.axis->alignment() == Qt::AlignBottom) {
                setLabelTextProperties(textItem, label, false,
                                       QQuickText::HAlignment::AlignLeft,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, label, false,
                                       QQuickText::HAlignment::AlignRight,
                                       QQuickText::VAlignment::AlignVCenter,
                                       axis->textElideMode());
            }
            textItem->setHeight(0);
            textItem->setVisible(true);
        } else {
            textItem->setVisible(false);
        }
    }
}

void AxisRenderer::updateDateTimeXAxisLabels(AxisProperties &ax, const QRectF rect)
{
    auto axis = qobject_cast<QDateTimeAxis *>(ax.axis);
    if (!axis)
        return;

    qint64 maxDate = ax.maxValue;
    qint64 minDate = ax.minValue;
    int dateTimeSize = ax.minLabel + 1;
    qint64 segment = (maxDate - minDate)
                   / ax.minLabel;

    qint64 anchor = (minDate / segment) * segment;

    // See if we need more text items
    updateAxisLabelItems(ax.textItems, dateTimeSize, axis->labelDelegate());
    Q_TRACE_SCOPE(QGraphs2DAxisRendererUpdateDateTimeXAxisLabels);

    for (auto i = 0; i < dateTimeSize; ++i) {
        auto &textItem = ax.textItems[i];
        if (axis->isVisible() && axis->labelsVisible()) {
            float posY = rect.y() + ax.y;
            textItem->setY(posY);
            float textItemWidth = 20;
            float posX = rect.x() + (((float) i) * ax.stepPx) - ax.displacement;
            const double titleMargin = 0.01;
            if ((posX - titleMargin) > (rect.width() + rect.x())
                || (posX + titleMargin) < rect.x()) {
                // Hide text item which are outside the axis area
                textItem->setVisible(false);
                continue;
            }
            // Take text size into account only after hiding
            posX += ax.x - 0.5 * textItemWidth;
            textItem->setX(posX);
            textItem->setWidth(textItemWidth);
            textItem->setRotation(axis->labelsAngle());
            auto date = QDateTime::fromMSecsSinceEpoch(anchor + (segment * i));
            QString label = date.toString(axis->labelFormat());
            if (ax.axis->alignment() == Qt::AlignTop || ax.axis->alignment() == Qt::AlignLeft) {
                setLabelTextProperties(textItem, label, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignBottom,
                                       axis->textElideMode());
            } else {
                setLabelTextProperties(textItem, label, true,
                                       QQuickText::HAlignment::AlignHCenter,
                                       QQuickText::VAlignment::AlignTop,
                                       axis->textElideMode());
            }
            textItem->setHeight(rect.height());
            textItem->setVisible(true);
        } else {
            textItem->setVisible(false);
        }
    }
}

void AxisRenderer::createDragHandler()
{
    m_dragHandler = new QQuickDragHandler(this);
    m_dragHandler->setDragThreshold(10);
    m_dragHandler->setTarget(nullptr);
    connect(m_dragHandler,
            &QQuickDragHandler::translationChanged,
            this,
            &AxisRenderer::onTranslationChanged);
    connect(m_dragHandler, &QQuickDragHandler::grabChanged, this, &AxisRenderer::onGrabChanged);
}

void AxisRenderer::deleteDragHandler()
{
    disconnect(m_dragHandler,
               &QQuickDragHandler::translationChanged,
               this,
               &AxisRenderer::onTranslationChanged);
    disconnect(m_dragHandler, &QQuickDragHandler::grabChanged, this, &AxisRenderer::onGrabChanged);
    m_dragHandler->deleteLater();
}

// Calculate suitable major step based on range
double AxisRenderer::getValueStepsFromRange(double range)
{
    int digits = std::ceil(std::log10(range));
    double r = std::pow(10.0, -digits);
    r *= 10.0;
    double v = std::ceil(range * r) / r;
    double step = v * 0.1;
    // Step must always be bigger than 0
    step = qMax(0.0001, step);
    return step;
}

// Calculate suitable decimals amount based on range
int AxisRenderer::getValueDecimalsFromRange(double range)
{
    if (range <= 0)
        return 0;
    int decimals = std::ceil(std::log10(10.0 / range));
    // Decimals must always be at least 0
    decimals = qMax(0, decimals);
    return decimals;
}

QT_END_NAMESPACE

#include "moc_axisrenderer_p.cpp"
