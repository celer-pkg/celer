// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCore/qloggingcategory.h>
#include <QtQuickLayouts/private/qquickflexboxlayoutitem_p.h>
#include <yoga/YGNode.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickFlexLayoutItem, "qt.quick.flexlayouts.item")

static constexpr YGWrap QtToYGFlexboxWrap(QQuickFlexboxLayout::FlexboxWrap qtWrap) {
    switch (qtWrap) {
        case QQuickFlexboxLayout::NoWrap: return YGWrap::YGWrapNoWrap;
        case QQuickFlexboxLayout::Wrap: return YGWrap::YGWrapWrap;
        case QQuickFlexboxLayout::WrapReverse: return YGWrap::YGWrapWrapReverse;
        default: {
            qWarning("Not a valid wrap");
            return YGWrap{};
        }
    }
};

static constexpr YGFlexDirection QtToYGFlexboxDirection(QQuickFlexboxLayout::FlexboxDirection qtDirection) {
    switch (qtDirection) {
        case QQuickFlexboxLayout::Column: return YGFlexDirection::YGFlexDirectionColumn;
        case QQuickFlexboxLayout::ColumnReverse: return YGFlexDirection::YGFlexDirectionColumnReverse;
        case QQuickFlexboxLayout::Row: return YGFlexDirection::YGFlexDirectionRow;
        case QQuickFlexboxLayout::RowReverse: return YGFlexDirection::YGFlexDirectionRowReverse;
        default: {
            qWarning("Not a valid direction");
            return YGFlexDirection{};
        }
    }
};

static constexpr YGAlign QtToYGFlexboxAlignment(QQuickFlexboxLayout::FlexboxAlignment qtAlignment) {
    switch (qtAlignment) {
        case QQuickFlexboxLayout::AlignAuto: return YGAlign::YGAlignAuto;
        case QQuickFlexboxLayout::AlignStart: return YGAlign::YGAlignFlexStart;
        case QQuickFlexboxLayout::AlignCenter: return YGAlign::YGAlignCenter;
        case QQuickFlexboxLayout::AlignEnd: return YGAlign::YGAlignFlexEnd;
        case QQuickFlexboxLayout::AlignStretch: return YGAlign::YGAlignStretch;
        case QQuickFlexboxLayout::AlignBaseline: return YGAlign::YGAlignBaseline;
        case QQuickFlexboxLayout::AlignSpaceBetween: return YGAlign::YGAlignSpaceBetween;
        case QQuickFlexboxLayout::AlignSpaceAround: return YGAlign::YGAlignSpaceAround;
        case QQuickFlexboxLayout::AlignSpaceEvenly: {
            return YGAlign{};
        }
        default: {
            qWarning("Not a valid alignment");
            return YGAlign{};
        }
    }
};

static constexpr YGJustify QtToYGFlexboxJustify(QQuickFlexboxLayout::FlexboxJustify qtJustify) {
    switch (qtJustify) {
    case QQuickFlexboxLayout::JustifyStart: return YGJustify::YGJustifyFlexStart;
    case QQuickFlexboxLayout::JustifyCenter: return YGJustify::YGJustifyCenter;
        case QQuickFlexboxLayout::JustifyEnd: return YGJustify::YGJustifyFlexEnd;
        case QQuickFlexboxLayout::JustifySpaceBetween: return YGJustify::YGJustifySpaceBetween;
        case QQuickFlexboxLayout::JustifySpaceAround: return YGJustify::YGJustifySpaceAround;
        case QQuickFlexboxLayout::JustifySpaceEvenly: return YGJustify::YGJustifySpaceEvenly;
        default: {
            qWarning("Not a valid justify");
            return YGJustify{};
        }
    }
};

static constexpr YGEdge QtToYGFlexboxEdge(QQuickFlexboxLayout::FlexboxEdge qtEdge) {
    switch (qtEdge) {
        case QQuickFlexboxLayout::EdgeLeft: return YGEdge::YGEdgeLeft;
        case QQuickFlexboxLayout::EdgeRight: return YGEdge::YGEdgeRight;
        case QQuickFlexboxLayout::EdgeTop: return YGEdge::YGEdgeTop;
        case QQuickFlexboxLayout::EdgeBottom: return YGEdge::YGEdgeBottom;
        case QQuickFlexboxLayout::EdgeAll: return YGEdge::YGEdgeAll;
        default: {
            qWarning("Not a valid edge");
            return YGEdge{};
        }
    }
};

static constexpr YGGutter QtToYGFlexboxGap(QQuickFlexboxLayout::FlexboxGap qtGap) {
    switch (qtGap) {
        case QQuickFlexboxLayout::GapRow: return YGGutter::YGGutterRow;
        case QQuickFlexboxLayout::GapColumn: return YGGutter::YGGutterColumn;
        case QQuickFlexboxLayout::GapAll: return YGGutter::YGGutterAll;
        default: {
            qWarning("Not a valid gap");
            return YGGutter{};
        }
    }
};

QQuickFlexboxLayoutItem::QQuickFlexboxLayoutItem(QQuickItem *item)
    : m_item(item)
{
    Q_ASSERT(m_item != nullptr);
    m_yogaNode = YGNodeNew();
    resetDefault();
}

QQuickFlexboxLayoutItem::~QQuickFlexboxLayoutItem()
{
    YGNodeFree(m_yogaNode);
}

void QQuickFlexboxLayoutItem::setMinSize(const QSizeF &size)
{
    YGNodeStyleSetMinWidth(m_yogaNode, static_cast<float>(size.width()));
    YGNodeStyleSetMinHeight(m_yogaNode, static_cast<float>(size.height()));
}

void QQuickFlexboxLayoutItem::setSize(const QSizeF &size)
{
    YGNodeStyleSetWidth(m_yogaNode, static_cast<float>(size.width()));
    YGNodeStyleSetHeight(m_yogaNode, static_cast<float>(size.height()));
}

void QQuickFlexboxLayoutItem::setWidth(const qreal &width)
{
    YGNodeStyleSetWidth(m_yogaNode, static_cast<float>(width));
}

void QQuickFlexboxLayoutItem::setHeight(const qreal &height)
{
    YGNodeStyleSetHeight(m_yogaNode, static_cast<float>(height));
}

void QQuickFlexboxLayoutItem::setMaxSize(const QSizeF &size)
{
    YGNodeStyleSetMaxWidth(m_yogaNode, static_cast<float>(size.width()));
    YGNodeStyleSetMaxHeight(m_yogaNode, static_cast<float>(size.height()));
}

void QQuickFlexboxLayoutItem::setFlexBasis(const qreal value, bool reset)
{
    YGNodeStyleSetFlexBasis(m_yogaNode, reset ? qQNaN() : value);
}

bool QQuickFlexboxLayoutItem::isFlexBasisUndefined() const
{
    float value = YGNodeStyleGetFlexBasis(m_yogaNode).value;
    return (value == YGUndefined || qt_is_nan(value));
}

void QQuickFlexboxLayoutItem::setItemGrowAlongMainAxis(const qreal value)
{
    YGNodeStyleSetFlexGrow(m_yogaNode, value);
}

void QQuickFlexboxLayoutItem::setItemShrinkAlongMainAxis(const qreal value)
{
    YGNodeStyleSetFlexShrink(m_yogaNode, value);
}

void QQuickFlexboxLayoutItem::setItemStretchAlongCrossSection()
{
    YGNodeStyleSetAlignSelf(m_yogaNode, YGAlignStretch);
}

void QQuickFlexboxLayoutItem::setFlexDirection(QQuickFlexboxLayout::FlexboxDirection direction)
{
    YGNodeStyleSetFlexDirection(m_yogaNode, QtToYGFlexboxDirection(direction));
}

void QQuickFlexboxLayoutItem::setFlexWrap(QQuickFlexboxLayout::FlexboxWrap wrap)
{
    YGNodeStyleSetFlexWrap(m_yogaNode, QtToYGFlexboxWrap(wrap));
}

void QQuickFlexboxLayoutItem::setFlexAlignItemsProperty(QQuickFlexboxLayout::FlexboxAlignment align)
{
    YGNodeStyleSetAlignItems(m_yogaNode, QtToYGFlexboxAlignment(align));
}

void QQuickFlexboxLayoutItem::setFlexAlignContentProperty(QQuickFlexboxLayout::FlexboxAlignment align)
{
    YGNodeStyleSetAlignContent(m_yogaNode, QtToYGFlexboxAlignment(align));
}

void QQuickFlexboxLayoutItem::setFlexJustifyContentProperty(QQuickFlexboxLayout::FlexboxJustify justify)
{
    YGNodeStyleSetJustifyContent(m_yogaNode, QtToYGFlexboxJustify(justify));
}

void QQuickFlexboxLayoutItem::setFlexAlignSelfProperty(QQuickFlexboxLayout::FlexboxAlignment align)
{
    YGNodeStyleSetAlignSelf(m_yogaNode, QtToYGFlexboxAlignment(align));
}

void QQuickFlexboxLayoutItem::setFlexMargin(QQuickFlexboxLayout::FlexboxEdge edge, const qreal value)
{
    YGNodeStyleSetMargin(m_yogaNode, QtToYGFlexboxEdge(edge), value);
}

void QQuickFlexboxLayoutItem::setFlexPadding(QQuickFlexboxLayout::FlexboxEdge edge, const qreal value)
{
    YGNodeStyleSetPadding(m_yogaNode, QtToYGFlexboxEdge(edge), value);
}

void QQuickFlexboxLayoutItem::setFlexGap(QQuickFlexboxLayout::FlexboxGap gap, const qreal value)
{
    YGNodeStyleSetGap(m_yogaNode, QtToYGFlexboxGap(gap), value);
}

bool QQuickFlexboxLayoutItem::isItemStreched() const
{
    return ((YGNodeStyleGetAlignSelf(m_yogaNode) == YGAlignStretch) ||
            ((YGNodeStyleGetAlignSelf(m_yogaNode) == YGAlignAuto) &&
             (YGNodeStyleGetAlignItems(m_yogaNode->getParent()) == YGAlignStretch)));
}

void QQuickFlexboxLayoutItem::inheritItemStretchAlongCrossSection()
{
    YGNodeStyleSetAlignSelf(m_yogaNode, YGAlignAuto);
}

void QQuickFlexboxLayoutItem::resetMargins()
{
    YGNodeStyleSetMargin(m_yogaNode, YGEdgeLeft, 0);
    YGNodeStyleSetMargin(m_yogaNode, YGEdgeTop, 0);
    YGNodeStyleSetMargin(m_yogaNode, YGEdgeRight, 0);
    YGNodeStyleSetMargin(m_yogaNode, YGEdgeBottom, 0);
}

void QQuickFlexboxLayoutItem::resetPaddings()
{
    YGNodeStyleSetPadding(m_yogaNode, YGEdgeAll, 0);
}

void QQuickFlexboxLayoutItem::resetSize()
{
    YGNodeStyleSetWidth(m_yogaNode, YGUndefined);
    YGNodeStyleSetHeight(m_yogaNode, YGUndefined);
}

QPoint QQuickFlexboxLayoutItem::position() const
{
    return QPoint(YGNodeLayoutGetLeft(m_yogaNode), YGNodeLayoutGetTop(m_yogaNode));
}

QSizeF QQuickFlexboxLayoutItem::size() const
{
    return QSizeF(YGNodeLayoutGetWidth(m_yogaNode), YGNodeLayoutGetHeight(m_yogaNode));
}

void QQuickFlexboxLayoutItem::insertChild(QQuickFlexboxLayoutItem *child, int index)
{
    YGNodeInsertChild(m_yogaNode, child->yogaItem(), index);
    // TODO: We may need this only for the text node?
    // YGNodeSetMeasureFunc(m_yogaNode, &QQuickFlexboxLayoutItem::measureFunc);
}

void QQuickFlexboxLayoutItem::resetDefault()
{
    // Context object is required here for callback functionality
    // For instance, the measurement function would be called to determine the
    // layout size
    YGNodeSetContext(m_yogaNode, this);
    resetMargins();
    resetPaddings();
    YGNodeStyleSetFlexBasis(m_yogaNode, YGUndefined);
}

bool QQuickFlexboxLayoutItem::hasMeasureFunc() const
{
    return YGNodeHasMeasureFunc(m_yogaNode);
}

void QQuickFlexboxLayoutItem::resetMeasureFunc()
{
    YGNodeSetMeasureFunc(m_yogaNode, nullptr);
}

SizeHints &QQuickFlexboxLayoutItem::cachedItemSizeHints() const
{
    return m_cachedSizeHint;
}

void QQuickFlexboxLayoutItem::computeLayout(const QSizeF &size)
{
    // Consider either NaN or Inf as YGUndefined
    const float width = (qt_is_nan(size.width()) || qt_is_inf(size.width()) ||
                         !size.width()) ? YGUndefined : size.width();
    const float height = (qt_is_nan(size.height()) || qt_is_inf(size.height()) ||
                          !size.height()) ? YGUndefined : size.height();
    YGNodeCalculateLayout(m_yogaNode, width, height, YGDirectionLTR);
}

QT_END_NAMESPACE
