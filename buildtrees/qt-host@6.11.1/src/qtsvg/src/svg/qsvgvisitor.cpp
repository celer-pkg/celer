// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgvisitor_p.h"

QT_BEGIN_NAMESPACE

void QSvgVisitor::traverse(const QSvgStructureNode *node)
{
    switch (node->type()) {
    case QSvgNode::Switch:
        if (!visitSwitchNodeStart(static_cast<const QSvgSwitch *>(node)))
            return;
        break;
    case QSvgNode::Doc:
        if (!visitDocumentNodeStart(static_cast<const QSvgDocument *>(node)))
            return;
        break;
    case QSvgNode::Defs:
        if (!visitDefsNodeStart(static_cast<const QSvgDefs *>(node)))
            return;
        break;
    case QSvgNode::Group:
        if (!visitGroupNodeStart(static_cast<const QSvgG *>(node)))
            return;
        break;
    case QSvgNode::Mask:
        if (!visitMaskNodeStart(static_cast<const QSvgMask *>(node)))
            return;
        break;
    case QSvgNode::Symbol:
        if (!visitSymbolNodeStart(static_cast<const QSvgSymbol *>(node)))
            return;
        break;
    case QSvgNode::Filter:
        if (!visitFilterNodeStart(static_cast<const QSvgFilterContainer *>(node)))
            return;
        break;
    case QSvgNode::Marker:
        if (!visitMarkerNodeStart(static_cast<const QSvgMarker *>(node)))
            return;
        break;
    case QSvgNode::Pattern:
        if (!visitPatternNodeStart(static_cast<const QSvgPattern *>(node)))
            return;
        break;
    case QSvgNode::FeMerge:
    case QSvgNode::FeMergenode:
    case QSvgNode::FeColormatrix:
    case QSvgNode::FeGaussianblur:
    case QSvgNode::FeOffset:
    case QSvgNode::FeComposite:
    case QSvgNode::FeFlood:
    case QSvgNode::FeBlend:
        if (!visitFeFilterPrimitiveNodeStart(static_cast<const QSvgFeFilterPrimitive *>(node)))
            return;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    for (auto &child : node->renderers())
        traverse(child.get());

    switch (node->type()) {
    case QSvgNode::Switch:
        visitSwitchNodeEnd(static_cast<const QSvgSwitch *>(node));
        break;
    case QSvgNode::Doc:
        visitDocumentNodeEnd(static_cast<const QSvgDocument *>(node));
        break;
    case QSvgNode::Defs:
        visitDefsNodeEnd(static_cast<const QSvgDefs *>(node));
        break;
    case QSvgNode::Group:
        visitGroupNodeEnd(static_cast<const QSvgG *>(node));
        break;
    case QSvgNode::Mask:
        visitMaskNodeEnd(static_cast<const QSvgMask *>(node));
        break;
    case QSvgNode::Symbol:
        visitSymbolNodeEnd(static_cast<const QSvgSymbol *>(node));
        break;
    case QSvgNode::Filter:
        visitFilterNodeEnd(static_cast<const QSvgFilterContainer *>(node));
        break;
    case QSvgNode::Marker:
        visitMarkerNodeEnd(static_cast<const QSvgMarker *>(node));
        break;
    case QSvgNode::Pattern:
        visitPatternNodeEnd(static_cast<const QSvgPattern *>(node));
        break;
    case QSvgNode::FeMerge:
    case QSvgNode::FeMergenode:
    case QSvgNode::FeColormatrix:
    case QSvgNode::FeGaussianblur:
    case QSvgNode::FeOffset:
    case QSvgNode::FeComposite:
    case QSvgNode::FeFlood:
    case QSvgNode::FeBlend:
        visitFeFilterPrimitiveNodeEnd(static_cast<const QSvgFeFilterPrimitive *>(node));
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

void QSvgVisitor::traverse(const QSvgNode *node)
{
    switch (node->type()) {
    case QSvgNode::Switch:
    case QSvgNode::Doc:
    case QSvgNode::Defs:
    case QSvgNode::Group:
    case QSvgNode::Mask:
    case QSvgNode::Symbol:
    case QSvgNode::Filter:
    case QSvgNode::FeMerge:
    case QSvgNode::FeMergenode:
    case QSvgNode::FeColormatrix:
    case QSvgNode::FeGaussianblur:
    case QSvgNode::FeOffset:
    case QSvgNode::FeComposite:
    case QSvgNode::FeFlood:
    case QSvgNode::FeBlend:
    case QSvgNode::Marker:
    case QSvgNode::Pattern:
        traverse(static_cast<const QSvgStructureNode *>(node));
        break;
    case QSvgNode::AnimateColor:
    case QSvgNode::AnimateTransform:
        visitAnimateNode(static_cast<const QSvgAnimateNode *>(node));
        break;
    case QSvgNode::Circle:
    case QSvgNode::Ellipse:
        visitEllipseNode(static_cast<const QSvgEllipse *>(node));
        break;
    case QSvgNode::Image:
        visitImageNode(static_cast<const QSvgImage *>(node));
        break;
    case QSvgNode::Line:
        visitLineNode(static_cast<const QSvgLine *>(node));
        break;
    case QSvgNode::Path:
        visitPathNode(static_cast<const QSvgPath *>(node));
        break;
    case QSvgNode::Polygon:
        visitPolygonNode(static_cast<const QSvgPolygon *>(node));
        break;
    case QSvgNode::Polyline:
        visitPolylineNode(static_cast<const QSvgPolyline *>(node));
        break;
    case QSvgNode::Rect:
        visitRectNode(static_cast<const QSvgRect *>(node));
        break;
    case QSvgNode::Text:
    case QSvgNode::Textarea:
        visitTextNode(static_cast<const QSvgText *>(node));
        break;
    case QSvgNode::Tspan:
        visitTspanNode(static_cast<const QSvgTspan *>(node));
        break;
    case QSvgNode::Use:
        visitUseNode(static_cast<const QSvgUse *>(node));
        break;
    case QSvgNode::Video:
        visitVideoNode(static_cast<const QSvgVideo *>(node));
        break;

        // Enum values that don't have any QSvgNode classes yet:
    case QSvgNode::FeUnsupported:
        qDebug() << "Unhandled type in switch" << node->type();
        break;
    }
}

QT_END_NAMESPACE
