// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qcustomseries.h>
#include <private/axisrenderer_p.h>
#include <private/charthelpers_p.h>
#include <private/customrenderer_p.h>
#include <private/qabstractseries_p.h>
#include <private/qcustomseriesdata_p.h>
#include <private/qgraphsview_p.h>
#include <QQmlIncubator>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
               "QT_BEGIN_NAMESPACE" \
               "class CustomRenderer;" \
               "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs2DCustomRendererHandlePolish_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DCustomRendererHandlePolish_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DCustomRendererAfterPolish_entry, int cleanupSeriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DCustomRendererAfterPolish_exit);

class DataItemIncubator : public QQmlIncubator
{
public:
    DataItemIncubator(CustomRenderer *renderer) : m_renderer(renderer) {};

private:
    void setInitialState(QObject *object) override
    {
        auto item = qobject_cast<QQuickItem *>(object);
        item->setParent(m_renderer);
        item->setParentItem(m_renderer);
        recursivelySetParents(item);
    }

    void recursivelySetParents(QQuickItem *item)
    {
        for (const auto &child : item->children()) {
            auto childItem = qobject_cast<QQuickItem *>(child);
            if (childItem) {
                childItem->setParentItem(item);
                recursivelySetParents(childItem);
            }
        }
    }

    CustomRenderer *m_renderer = nullptr;
};

CustomRenderer::CustomRenderer(QGraphsView *graph, bool clipPlotArea)
    : QQuickItem(graph)
    , m_graph(graph)
{
    setFlag(QQuickItem::ItemHasContents);
    setClip(clipPlotArea);
}

CustomRenderer::~CustomRenderer()
{
    qDeleteAll(m_groups);
}

qreal CustomRenderer::mapX(AxisRenderer *axisRenderer, QCustomSeries *series, qreal x)
{
    auto &axisX = axisRenderer->getAxisX(series);
    auto flipX = axisX.maxValue < axisX.minValue ? -1 : 1;
    return m_areaWidth * flipX * x * m_maxHorizontal - m_horizontalOffset;
}

qreal CustomRenderer::mapY(AxisRenderer *axisRenderer, QCustomSeries *series, qreal y)
{
    auto &axisY = axisRenderer->getAxisY(series);

    if (m_graph->orientation() != Qt::Vertical)
        y = axisY.maxValue - y;

    auto flipY = axisY.maxValue < axisY.minValue ? -1 : 1;
    return m_areaHeight - m_areaHeight * flipY * y * m_maxVertical + m_verticalOffset;
}

void CustomRenderer::hideDelegates(QCustomSeries *series)
{
    auto group = m_groups.value(series);
    if (group && group->currentDataItem) {
        for (int i = 0; i < group->dataItems.size(); ++i) {
            auto *dataItem = group->dataItems[i];
            dataItem->setVisible(false);
        }
    }
}

void CustomRenderer::handlePolish(QCustomSeries *series)
{
    Q_TRACE(QGraphs2DCustomRendererHandlePolish_entry);

    auto theme = m_graph->theme();
    if (!theme) {
        qCCritical(lcCritical2D, "Theme not found");
        return;
    }

    if (!m_graph->m_axisRenderer) {
        qCCritical(lcCritical2D, "Axis renderer not found.");
        return;
    }

    if (series->dataItems().isEmpty()) {
        auto group = m_groups.value(series);

        if (group) {
            for (auto m : std::as_const(group->dataItems))
                m->deleteLater();

            group->dataItems.clear();
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
        DataGroup *group = new DataGroup();
        group->series = series;
        m_groups.insert(series, group);
    }

    auto group = m_groups.value(series);

    qsizetype dataItemCount = series->dataItems().size();

    if (series->delegate())
        group->currentDataItem = series->delegate();

    if (group->currentDataItem != group->previousDataItem) {
        for (auto &&dataItem : group->dataItems)
            dataItem->deleteLater();
        group->dataItems.clear();
    }
    group->previousDataItem = group->currentDataItem;

    if (group->currentDataItem) {
        qsizetype dataItemVisualCount = group->dataItems.size();
        if (dataItemVisualCount < dataItemCount) {
            for (qsizetype i = dataItemVisualCount; i < dataItemCount; ++i) {
                DataItemIncubator incubator(this);
                group->currentDataItem->create(
                    incubator, group->currentDataItem->creationContext());
                incubator.forceCompletion();
                group->dataItems << qobject_cast<QQuickItem *>(incubator.object());
            }
        } else if (dataItemVisualCount > dataItemCount) {
            for (qsizetype i = dataItemCount; i < dataItemVisualCount; ++i)
                group->dataItems[i]->deleteLater();
            group->dataItems.resize(dataItemCount);
        }
    } else if (group->dataItems.size() > 0) {
        for (auto &&dataItem : group->dataItems)
            dataItem->deleteLater();
        group->dataItems.clear();
    }

    if (series->isVisible()) {
        for (int i = 0; i < group->dataItems.size(); ++i) {
            auto item = group->dataItems.at(i);
            series->updateDelegate(item, i);
            item->setZ(group->series->zValue());
        }
    } else {
        hideDelegates(series);
    }

    if (m_plotArea.width() != m_areaWidth || m_plotArea.height() != m_areaHeight) {
        m_plotArea.setWidth(m_areaWidth);
        m_plotArea.setHeight(m_areaHeight);
        emit plotAreaChanged();
    }

    Q_TRACE(QGraphs2DCustomRendererHandlePolish_exit);
}

void CustomRenderer::afterPolish(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_TRACE(QGraphs2DCustomRendererAfterPolish_entry, static_cast<int>(cleanupSeries.count()));

    for (auto series : cleanupSeries) {
        auto customSeries = qobject_cast<QCustomSeries *>(series);
        if (customSeries && m_groups.contains(customSeries)) {
            auto group = m_groups.value(customSeries);

            for (auto dataItem : std::as_const(group->dataItems))
                dataItem->deleteLater();

            delete group;
            m_groups.remove(customSeries);
        }
    }

    Q_TRACE(QGraphs2DCustomRendererAfterPolish_exit);
}

void CustomRenderer::updateSeries(QCustomSeries *series)
{
    Q_UNUSED(series);
}

void CustomRenderer::afterUpdate(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_UNUSED(cleanupSeries);
}

QRectF CustomRenderer::plotArea() const
{
    return m_plotArea;
}

QT_END_NAMESPACE

#include "moc_customrenderer_p.cpp"
