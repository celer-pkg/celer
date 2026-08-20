// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick/private/qquickrectangle_p.h>
#include <QtQuick/private/qquicktaphandler_p.h>
#include <QtGraphs/qbarseries.h>
#include <QtGraphs/qbarset.h>
#include <private/barsrenderer_p.h>
#include <private/axisrenderer_p.h>
#include <private/qbarseries_p.h>
#include <private/qgraphsview_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
              "class BarsRenderer;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateVerticalBars_entry, int setCount, int valuesPerSet, int barSeriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateVerticalBars_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateupdateHorizontalBars_entry, int setCount, int valuesPerSet, int barSeriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateupdateHorizontalBars_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateValueLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateValueLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateComponents_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DBarsRendererUpdateComponents_exit);

static const char* TAG_BAR_COLOR = "barColor";
static const char* TAG_BAR_BORDER_COLOR = "barBorderColor";
static const char* TAG_BAR_BORDER_WIDTH = "barBorderWidth";
static const char* TAG_BAR_SELECTED = "barSelected";
static const char* TAG_BAR_VALUE = "barValue";
static const char* TAG_BAR_LABEL = "barLabel";
static const char* TAG_BAR_INDEX = "barIndex";

BarsRenderer::BarsRenderer(QGraphsView *graph, bool clipPlotArea)
    : QQuickItem(graph)
    , m_graph(graph)
{
    setFlag(QQuickItem::ItemHasContents);
    setClip(clipPlotArea);

    m_tapHandler = new QQuickTapHandler(this);
    connect(m_tapHandler, &QQuickTapHandler::singleTapped, this, &BarsRenderer::onSingleTapped);
    connect(m_tapHandler, &QQuickTapHandler::doubleTapped, this, &BarsRenderer::onDoubleTapped);
    connect(m_tapHandler, &QQuickTapHandler::pressedChanged, this, &BarsRenderer::onPressedChanged);
}

BarsRenderer::~BarsRenderer() {}

// Returns color in this order:
// 1) QBarSet::color if that is defined (alpha > 0).
// 2) QBarSeries::seriesColors at index if that is defined.
// 3) QGraphsTheme::seriesColors at index.
// 4) Black if seriesColors is empty.
QColor BarsRenderer::getSetColor(QBarSeries *series, QBarSet *set, qsizetype barSeriesIndex)
{
    const auto &seriesColors = !series->seriesColors().isEmpty()
            ? series->seriesColors() : m_graph->theme()->seriesColors();
    if (seriesColors.isEmpty())
        return QColorConstants::Black;
    qsizetype index = m_colorIndex + barSeriesIndex;
    index = index % seriesColors.size();
    QColor color = set->color().alpha() != 0
            ? set->color()
            : seriesColors.at(index);
    return color;
}

QColor BarsRenderer::getSetSelectedColor(QBarSeries *series, QBarSet *set)
{
    Q_UNUSED(series);
    auto theme = m_graph->theme();
    QColor color = set->selectedColor().alpha() != 0
            ? set->selectedColor()
            : theme->singleHighlightColor();
    return color;
}

QColor BarsRenderer::getSetBorderColor(QBarSeries *series, QBarSet *set, qsizetype barSeriesIndex)
{
    const auto &borderColors = !series->borderColors().isEmpty()
            ? series->borderColors() : m_graph->theme()->borderColors();
    if (borderColors.isEmpty())
        return QColorConstants::Black;
    qsizetype index = m_colorIndex + barSeriesIndex;
    index = index % borderColors.size();
    QColor color = set->borderColor().alpha() != 0
            ? set->borderColor()
            : borderColors.at(index);
    return color;
}

qreal BarsRenderer::getSetBorderWidth(QBarSeries *series, QBarSet *set)
{
    Q_UNUSED(series);
    auto theme = m_graph->theme();
    qreal borderWidth = set->borderWidth();
    if (qFuzzyCompare(borderWidth, qreal(-1.0)))
        borderWidth = theme->borderWidth();
    return borderWidth;
}

QString BarsRenderer::generateLabelText(QBarSeries *series, qreal value)
{
    static const QString valueTag(QLatin1String("@value"));
    QString valueString = QString::number(value, 'f', series->labelsPrecision());
    QString valueLabel;
    if (series->labelsFormat().isEmpty()) {
        valueLabel = valueString;
    } else {
        valueLabel = series->labelsFormat();
        valueLabel.replace(valueTag, valueString);
    }
    return valueLabel;
}

void BarsRenderer::positionLabelItem(QBarSeries *series, QQuickText *textItem, const BarSeriesData &d)
{
    auto pos = series->labelsPosition();
    const bool vertical = m_graph->orientation() == Qt::Orientation::Vertical;
    const float w = textItem->contentWidth() + series->labelsMargin() * 2;
    const float h = textItem->contentHeight() + series->labelsMargin() * 2;
    textItem->setWidth(w);
    textItem->setHeight(h);
    textItem->setHAlign(QQuickText::HAlignment::AlignHCenter);
    textItem->setVAlign(QQuickText::VAlignment::AlignVCenter);
    if (pos == QBarSeries::LabelsPosition::Center) {
        textItem->setX(d.rect.x() + d.rect.width() * 0.5 - w * 0.5);
        textItem->setY(d.rect.y() + d.rect.height() * 0.5 - h * 0.5);
    } else if (pos == QBarSeries::LabelsPosition::InsideEnd) {
        if (vertical) {
            textItem->setX(d.rect.x() + d.rect.width() * 0.5 - w * 0.5);
            textItem->setY(d.rect.y());
        } else {
            textItem->setX(d.rect.x() + d.rect.width() - w);
            textItem->setY(d.rect.y() + d.rect.height() * 0.5 - h * 0.5);
        }
    } else if (pos == QBarSeries::LabelsPosition::InsideBase) {
        if (vertical) {
            textItem->setX(d.rect.x() + d.rect.width() * 0.5 - w * 0.5);
            textItem->setY(d.rect.y() + d.rect.height() - h);
        } else {
            textItem->setX(d.rect.x());
            textItem->setY(d.rect.y() + d.rect.height() * 0.5 - h * 0.5);
        }
    } else {
        // OutsideEnd
        if (vertical) {
            textItem->setX(d.rect.x() + d.rect.width() * 0.5 - w * 0.5);
            textItem->setY(d.rect.y() - h);
        } else {
            textItem->setX(d.rect.x() + d.rect.width());
            textItem->setY(d.rect.y() + d.rect.height() * 0.5 - h * 0.5);
        }
    }
    textItem->update();
}

void BarsRenderer::updateComponents(QBarSeries *series)
{
    int barIndex = 0;
    auto &seriesData = m_seriesData[series];
    auto &barItems = m_barItems[series];
    Q_TRACE(QGraphs2DBarsRendererUpdateComponents_entry);
    for (auto i = seriesData.cbegin(), end = seriesData.cend(); i != end; ++i) {
        if (barItems.size() <= barIndex) {
            QQuickItem *item = nullptr;
            // Create more components as needed
            if (series->barDelegate()) {
                item = qobject_cast<QQuickItem *>(
                        series->barDelegate()->create(series->barDelegate()->creationContext()));
            }
            if (!item)
                item = new QQuickRectangle();
            item->setParent(this);
            item->setParentItem(this);
            barItems << item;
        }
        if (barItems.size() > barIndex) {
            BarSeriesData d = *i;
            if (series->barDelegate()) {
                // Set custom bar components
                auto &barItem = barItems[barIndex];
                barItem->setX(d.rect.x());
                barItem->setY(d.rect.y());
                barItem->setZ(series->zValue());
                barItem->setWidth(d.rect.width());
                barItem->setHeight(d.rect.height());
                barItem->setVisible(series->isVisible());
                // Check for specific dynamic properties
                if (barItem->property(TAG_BAR_COLOR).isValid())
                    barItem->setProperty(TAG_BAR_COLOR, d.color);
                if (barItem->property(TAG_BAR_BORDER_COLOR).isValid())
                    barItem->setProperty(TAG_BAR_BORDER_COLOR, d.borderColor);
                if (barItem->property(TAG_BAR_BORDER_WIDTH).isValid())
                    barItem->setProperty(TAG_BAR_BORDER_WIDTH, d.borderWidth);
                if (barItem->property(TAG_BAR_SELECTED).isValid())
                    barItem->setProperty(TAG_BAR_SELECTED, d.isSelected);
                if (barItem->property(TAG_BAR_VALUE).isValid())
                    barItem->setProperty(TAG_BAR_VALUE, d.value);
                if (barItem->property(TAG_BAR_LABEL).isValid())
                    barItem->setProperty(TAG_BAR_LABEL, d.label);
                if (barItem->property(TAG_BAR_INDEX).isValid())
                    barItem->setProperty(TAG_BAR_INDEX, barIndex);
            } else {
                // Set default rectangle bars
                auto barItem = qobject_cast<QQuickRectangle *>(barItems[barIndex]);
                if (barItem) {
                    barItem->setX(d.rect.x());
                    barItem->setY(d.rect.y());
                    barItem->setZ(series->zValue());
                    barItem->setWidth(d.rect.width());
                    barItem->setHeight(d.rect.height());
                    barItem->setVisible(series->isVisible());
                    barItem->setColor(d.color);
                    barItem->border()->setColor(d.borderColor);
                    barItem->border()->setWidth(d.borderWidth);
                    barItem->setRadius(4.0);
                }
            }
        }
        barIndex++;
    }
    Q_TRACE(QGraphs2DBarsRendererUpdateComponents_exit);
}

void BarsRenderer::updateValueLabels(QBarSeries *series)
{
    if (!series->barDelegate() && series->isVisible() && series->labelsVisible()) {
        // Update default value labels
        int barIndex = 0;
        auto &seriesData = m_seriesData[series];
        auto &labelTextItems = m_labelTextItems[series];
        Q_TRACE(QGraphs2DBarsRendererUpdateValueLabels_entry);
        for (auto i = seriesData.cbegin(), end = seriesData.cend(); i != end; ++i) {
            if (labelTextItems.size() <= barIndex) {
                // Create more label items as needed
                auto labelItem = new QQuickText(this);
                labelTextItems << labelItem;
            }
            if (labelTextItems.size() > barIndex) {
                // Set label item values
                auto &textItem = labelTextItems[barIndex];
                const auto d = *i;
                if (qFuzzyIsNull(d.value)) {
                    textItem->setVisible(false);
                } else {
                    textItem->setVisible(series->labelsVisible());
                    QString valueLabel = generateLabelText(series, d.value);
                    textItem->setText(valueLabel);
                    positionLabelItem(series, textItem, d);
                    QColor labelColor = d.labelColor;
                    if (labelColor.alpha() == 0) {
                        // TODO: Use graphs theme labels color.
                        labelColor = QColor(255, 255, 255);
                    }
                    textItem->setColor(labelColor);
                    textItem->setRotation(series->labelsAngle());
                }
            }
            barIndex++;
        }
        Q_TRACE(QGraphs2DBarsRendererUpdateValueLabels_exit);
    } else {
        // Hide all possibly existing label items
        auto &labelTextItems = m_labelTextItems[series];
        for (auto textItem : labelTextItems)
            textItem->setVisible(false);
    }
}

void calculateCategoryTotalValues(QBarSeries *series, QList<float> &totalValues, qsizetype valuesPerSet)
{
    totalValues.fill(0, valuesPerSet);
    auto barsets = series->barSets();
    for (auto s : std::as_const(barsets)) {
        QVariantList v = s->values();
        int setIndex = 0;
        for (const auto &variantValue : std::as_const(v)) {
            if (setIndex < totalValues.size())
                totalValues[setIndex] += qAbs(variantValue.toReal());
            setIndex++;
        }
    }
}

void BarsRenderer::updateVerticalBars(QBarSeries *series, qsizetype setCount,
                                      qsizetype valuesPerSet, int barSeriesIndex,
                                      int barSeriesCount)
{
    bool stacked = series->barsType() == QBarSeries::BarsType::Stacked
            || series->barsType() == QBarSeries::BarsType::StackedPercent;
    bool percent = series->barsType() == QBarSeries::BarsType::StackedPercent;
    // Bars area width & height
    float w = width();
    float h = height();
    // Max width of a bar if no separation between sets.
    float maxBarWidth = w / (setCount * valuesPerSet * barSeriesCount) - m_barMargin;
    if (stacked)
        maxBarWidth = w / (valuesPerSet * barSeriesCount);
    // Actual bar width.
    float barWidth = maxBarWidth * series->barWidth();
    // Helper to keep barsets centered when bar width is less than max width.
    float barCentering = (maxBarWidth - barWidth) * setCount * 0.5;
    if (stacked)
        barCentering = (maxBarWidth - barWidth) * 0.5;

    const float barSeriesOffset = ((float)barSeriesIndex / (valuesPerSet * barSeriesCount)) * w;

    auto &seriesData = m_seriesData[series];
    auto &rectNodesInputRects = m_rectNodesInputRects[series];
    // Clear the selection rects
    // These will be filled only if series is selectable
    rectNodesInputRects.clear();
    seriesData.clear();

    float seriesPos = barSeriesOffset;
    float posXInSet = 0;
    QList<float> positiveYListInSet;
    QList<float> negativeYListInSet;
    if (stacked) {
        positiveYListInSet.fill(0, valuesPerSet);
        negativeYListInSet.fill(0, valuesPerSet);
    }
    QList<float> totalValuesListInSet;
    if (percent)
        calculateCategoryTotalValues(series, totalValuesListInSet, valuesPerSet);
    int barIndexInSet = 0;
    int barSetIndex = 0;
    QList<QLegendData> legendDataList;
    auto barsets = series->barSets();
    Q_TRACE(QGraphs2DBarsRendererUpdateVerticalBars_entry, setCount, valuesPerSet, barSeriesCount);
    for (auto s : std::as_const(barsets)) {
        QVariantList v = s->values();
        qsizetype valuesCount = v.size();
        if (valuesCount == 0)
            continue;
        seriesPos = barSeriesOffset;
        barIndexInSet = 0;
        BarSelectionRect *barSelectionRect = nullptr;
        if (series->isSelectable() || series->isHoverable()) {
            rectNodesInputRects << BarSelectionRect();
            barSelectionRect = &rectNodesInputRects.last();
            barSelectionRect->barSet = s;
            barSelectionRect->series = series;
        }

        auto &axisY = m_graph->m_axisRenderer->getAxisY(series);

        QColor color = getSetColor(series, s, barSetIndex);
        QColor borderColor = getSetBorderColor(series, s, barSetIndex);
        qreal borderWidth = getSetBorderWidth(series, s);

        // Update legendData
        legendDataList.push_back({
                color,
                borderColor,
                s->label()
        });
        // Apply series opacity
        color.setAlpha(color.alpha() * series->opacity());
        borderColor.setAlpha(borderColor.alpha() * series->opacity());
        const auto selectedBars = s->selectedBars();
        for (const auto &variantValue : std::as_const(v)) {
            const float realValue = variantValue.toReal();
            float value = realValue * series->valuesMultiplier();
            if (percent) {
                if (auto totalValue = totalValuesListInSet.at(barIndexInSet))
                    value *= (100.0 / totalValue);
            }
            const bool isSelected = selectedBars.contains(barIndexInSet);
            double delta = axisY.maxValue - axisY.minValue;
            double maxValues = delta > 0 ? 1.0 / delta : 100.0;
            float barLength = qAbs(h * value * maxValues);
            float barY = h - barLength;
            float barX = seriesPos + posXInSet + barCentering;
            float offset = h * (- axisY.minValue) * series->valuesMultiplier() * maxValues;
            if (stacked) {
                barX = seriesPos + barCentering;
                if (value >= 0)
                    barY = h - barLength - offset - positiveYListInSet[barIndexInSet];
                else
                    barY = h - offset + negativeYListInSet[barIndexInSet];
            } else {
                barY -= offset;
                if (value < 0)
                    barY += barLength;
            }
            QRectF barRect(barX, barY, barWidth, barLength);
            if (barSelectionRect)
                barSelectionRect->rects << barRect;

            // Collect the series data
            BarSeriesData d;
            d.rect = barRect;
            d.color = isSelected ? getSetSelectedColor(series, s) : color;
            d.borderColor = borderColor;
            d.borderWidth = borderWidth;
            d.isSelected = isSelected;
            d.label = s->label();
            d.labelColor = s->labelColor();
            d.value = realValue;
            seriesData << d;

            if (stacked) {
                if (value >= 0)
                    positiveYListInSet[barIndexInSet] += barLength;
                else
                    negativeYListInSet[barIndexInSet] += barLength;
            }
            barIndexInSet++;
            seriesPos = ((float)barIndexInSet / valuesPerSet) * w + barSeriesOffset;
        }
        posXInSet += barWidth + m_barMargin;
        barSetIndex++;
    }
    Q_TRACE(QGraphs2DBarsRendererUpdateVerticalBars_exit);
    series->d_func()->setLegendData(legendDataList);
}

void BarsRenderer::updateHorizontalBars(QBarSeries *series, qsizetype setCount,
                                        qsizetype valuesPerSet, int barSeriesIndex,
                                        int barSeriesCount)
{
    bool stacked = series->barsType() == QBarSeries::BarsType::Stacked
            || series->barsType() == QBarSeries::BarsType::StackedPercent;
    bool percent = series->barsType() == QBarSeries::BarsType::StackedPercent;
    // Bars area width & height
    float w = width();
    float h = height();
    // Max width of a bar if no separation between sets.
    float maxBarWidth = h / (setCount * valuesPerSet * barSeriesCount) - m_barMargin;
    if (stacked)
        maxBarWidth = h / (valuesPerSet * barSeriesCount);
    // Actual bar width.
    float barWidth = maxBarWidth * series->barWidth();
    // Helper to keep barsets centered when bar width is less than max width.
    float barCentering = (maxBarWidth - barWidth) * setCount * 0.5;
    if (stacked)
        barCentering = (maxBarWidth - barWidth) * 0.5;

    const float barSeriesOffset = ((float)barSeriesIndex / (valuesPerSet * barSeriesCount)) * h;

    auto &seriesData = m_seriesData[series];
    auto &rectNodesInputRects = m_rectNodesInputRects[series];
    // Clear the selection rects
    // These will be filled only if series is selectable
    rectNodesInputRects.clear();
    seriesData.clear();

    float seriesPos = barSeriesOffset;
    float posYInSet = 0;
    QList<float> positiveXListInSet;
    QList<float> negativeXListInSet;
    if (stacked) {
        positiveXListInSet.fill(0, valuesPerSet);
        negativeXListInSet.fill(0, valuesPerSet);
    }
    QList<float> totalValuesListInSet;
    if (percent)
        calculateCategoryTotalValues(series, totalValuesListInSet, valuesPerSet);
    int barIndexInSet = 0;
    int barSetIndex = 0;
    QList<QLegendData> legendDataList;
    auto barsets = series->barSets();
    Q_TRACE(QGraphs2DBarsRendererUpdateupdateHorizontalBars_entry, setCount, valuesPerSet, barSeriesCount);
    for (auto s : std::as_const(barsets)) {
        QVariantList v = s->values();
        qsizetype valuesCount = v.size();
        if (valuesCount == 0)
            continue;
        seriesPos = barSeriesOffset;
        barIndexInSet = 0;
        BarSelectionRect *barSelectionRect = nullptr;
        if (series->isSelectable() || series->isHoverable()) {
            rectNodesInputRects << BarSelectionRect();
            barSelectionRect = &rectNodesInputRects.last();
            barSelectionRect->barSet = s;
            barSelectionRect->series = series;
        }

        auto &axisX = m_graph->m_axisRenderer->getAxisX(series);

        QColor color = getSetColor(series, s, barSetIndex);
        QColor borderColor = getSetBorderColor(series, s, barSetIndex);
        qreal borderWidth = getSetBorderWidth(series, s);
        // Update legendData
        legendDataList.push_back({
                color,
                borderColor,
                s->label()
        });
        // Apply series opacity
        color.setAlpha(color.alpha() * series->opacity());
        borderColor.setAlpha(borderColor.alpha() * series->opacity());
        const auto selectedBars = s->selectedBars();
        for (const auto &variantValue : std::as_const(v)) {
            const float realValue = variantValue.toReal();
            float value = realValue * series->valuesMultiplier();
            if (percent) {
                if (auto totalValue = totalValuesListInSet.at(barIndexInSet))
                    value *= (100.0 / totalValue);
            }
            const bool isSelected = selectedBars.contains(barIndexInSet);
            double delta = axisX.maxValue - axisX.minValue;
            double maxValues = delta > 0 ? 1.0 / delta : 100.0;
            float barLength = qAbs(w * value * maxValues);
            float barY = seriesPos + posYInSet + barCentering;
            float barX = 0;
            float offset = w * (- axisX.minValue) * series->valuesMultiplier() * maxValues;
            if (stacked) {
                barY = seriesPos + barCentering;
                if (value >= 0)
                    barX = offset + positiveXListInSet[barIndexInSet];
                else
                    barX = offset - barLength - negativeXListInSet[barIndexInSet];
            } else {
                barX += offset;
                if (value < 0)
                    barX -= barLength;
            }
            QRectF barRect(barX, barY, barLength, barWidth);
            if (barSelectionRect)
                barSelectionRect->rects << barRect;

            // Collect the series data
            BarSeriesData d;
            d.rect = barRect;
            d.color = isSelected ? getSetSelectedColor(series, s) : color;
            d.borderColor = borderColor;
            d.borderWidth = borderWidth;
            d.isSelected = isSelected;
            d.label = s->label();
            d.labelColor = s->labelColor();
            d.value = realValue;
            seriesData << d;

            if (stacked) {
                if (value >= 0)
                    positiveXListInSet[barIndexInSet] += barLength;
                else
                    negativeXListInSet[barIndexInSet] += barLength;
            }
            barIndexInSet++;
            seriesPos = ((float)barIndexInSet / valuesPerSet) * h + barSeriesOffset;
        }
        posYInSet += barWidth + m_barMargin;
        barSetIndex++;
    }
    Q_TRACE(QGraphs2DBarsRendererUpdateVerticalBars_exit);
    series->d_func()->setLegendData(legendDataList);
}

void BarsRenderer::handlePolish(QBarSeries *series, int barSeriesIndex, int barSeriesCount)
{
    auto theme = m_graph->theme();
    if (!theme) {
        qCCritical(lcCritical2D, "Theme not found.");
        return;
    }

    if (!m_graph->m_axisRenderer) {
        qCCritical(lcCritical2D, "Axis renderer not found.");
        return;
    }

    qsizetype setCount = series->barSets().size();
    auto &seriesData = m_seriesData[series];
    auto &barItems = m_barItems[series];
    auto &rectNodesInputRects = m_rectNodesInputRects[series];
    if (setCount == 0) {
        for (int i = 0; i < barItems.size(); i++)
            barItems[i]->deleteLater();
        barItems.clear();

        series->d_func()->clearLegendData();
        rectNodesInputRects.clear();
        seriesData.clear();
        return;
    }

    if (m_colorIndex < 0)
        m_colorIndex = m_graph->graphSeriesCount();
    m_graph->setGraphSeriesCount(m_colorIndex + setCount);

    if (series->barDelegateDirty() && !barItems.isEmpty()) {
        // Bars delegate has changed, so remove the old items.
        for (int i = 0; i < barItems.size(); i++)
            barItems[i]->deleteLater();
        barItems.clear();
        series->setBarDelegateDirty(false);
    }

    // Get bars values
    qsizetype valuesPerSet = series->barSets().constFirst()->values().size();
    if (m_graph->orientation() == Qt::Orientation::Vertical)
        updateVerticalBars(series, setCount, valuesPerSet, barSeriesIndex, barSeriesCount);
    else
        updateHorizontalBars(series, setCount, valuesPerSet, barSeriesIndex, barSeriesCount);
    updateComponents(series);
    updateValueLabels(series);

    // Remove additional components
    for (qsizetype i = barItems.size() - 1; i >= seriesData.size(); --i)
        barItems[i]->deleteLater();
    const auto range = barItems.size() - seriesData.size();
    if (range > 0)
        barItems.remove(seriesData.size(), range);
}

void BarsRenderer::updateSeries(QBarSeries *series)
{
    Q_UNUSED(series);
}

void BarsRenderer::afterUpdate(QList<QAbstractSeries *> &cleanupSeries)
{
    Q_UNUSED(cleanupSeries);
}

void BarsRenderer::afterPolish(QList<QAbstractSeries *> &cleanupSeries)
{
    for (auto &cleanupSerie : cleanupSeries) {
        auto series = qobject_cast<QBarSeries *>(cleanupSerie);
        if (series && m_barItems.contains(series)) {
            // Remove custom bar items
            auto &barItems = m_barItems[series];
            for (int i = 0; i < barItems.size(); i++)
                barItems[i]->deleteLater();
            barItems.clear();
            m_barItems.remove(series);
        }
        if (series && m_labelTextItems.contains(series)) {
            // Remove bar label items
            auto &labelTextItems = m_labelTextItems[series];
            for (int i = 0; i < labelTextItems.size(); i++)
                labelTextItems[i]->deleteLater();
            labelTextItems.clear();
            m_labelTextItems.remove(series);
        }
    }
}

bool BarsRenderer::handleHoverMove(QHoverEvent *event)
{
    bool handled = false;
    const QPointF &position = event->position();

    bool hovering = false;
    for (auto &rectNodesInputRects : m_rectNodesInputRects) {
        for (auto &barSelection : rectNodesInputRects) {
            int indexInSet = 0;

            for (auto &rect : barSelection.rects) {
                if (rect.contains(event->position().toPoint())) {
                    const QString &name = barSelection.series->name();
                    const QPointF point(indexInSet, barSelection.barSet->at(indexInSet));

                    if (!m_currentHoverSeries) {
                        m_currentHoverSeries = barSelection.series;
                        barSelection.series->setHovered(true);
                        emit barSelection.series->hoverEnter(name, position, point);
                    }

                    emit barSelection.series->hover(name, position, point);
                    hovering = true;
                    handled = true;
                }
                indexInSet++;
            }
        }
    }

    if (!hovering && m_currentHoverSeries) {
        m_currentHoverSeries->setHovered(false);
        emit m_currentHoverSeries->hoverExit(m_currentHoverSeries->name(), position);
        m_currentHoverSeries = nullptr;
        handled = true;
    }
    return handled;
}

void BarsRenderer::onSingleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    for (auto &rectNodesInputRects : m_rectNodesInputRects) {
        for (auto &barSelection : rectNodesInputRects) {
            if (!barSelection.series->isSelectable())
                continue;
            qsizetype indexInSet = 0;
            for (auto &rect : barSelection.rects) {
                if (rect.contains(eventPoint.position())) {
                    emit barSelection.series->clicked(indexInSet, barSelection.barSet);
                    return;
                }
                indexInSet++;
            }
        }
    }
}

void BarsRenderer::onDoubleTapped(QEventPoint eventPoint, Qt::MouseButton button)
{
    Q_UNUSED(button)

    for (auto &rectNodesInputRects : m_rectNodesInputRects) {
        for (auto &barSelection : rectNodesInputRects) {
            if (!barSelection.series->isSelectable())
                continue;
            qsizetype indexInSet = 0;
            for (auto &rect : barSelection.rects) {
                if (rect.contains(eventPoint.position())) {
                    emit barSelection.series->doubleClicked(indexInSet, barSelection.barSet);
                    return;
                }
                indexInSet++;
            }
        }
    }
}

void BarsRenderer::onPressedChanged()
{
    for (auto &rectNodesInputRects : m_rectNodesInputRects) {
        for (auto &barSelection : rectNodesInputRects) {
            if (!barSelection.series->isSelectable())
                continue;
            qsizetype indexInSet = 0;
            for (auto &rect : barSelection.rects) {
                if (rect.contains(m_tapHandler->point().position())) {
                    // TODO: Currently just toggling selection
                    if (m_tapHandler->isPressed()) {
                        QList<qsizetype> indexList = {indexInSet};
                        barSelection.barSet->toggleSelection(indexList);
                        emit barSelection.series->pressed(indexInSet, barSelection.barSet);
                    } else {
                        emit barSelection.series->released(indexInSet, barSelection.barSet);
                    }
                }
                indexInSet++;
            }
        }
    }
}

QT_END_NAMESPACE

#include "moc_barsrenderer_p.cpp"
