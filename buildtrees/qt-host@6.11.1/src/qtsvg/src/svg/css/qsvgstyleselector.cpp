// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgstyleselector_p.h"
#include <QtSvg/private/qsvgnode_p.h>
#include <QtSvg/private/qsvgstructure_p.h>

QT_BEGIN_NAMESPACE

QSvgStyleSelector::QSvgStyleSelector()
{
    nameCaseSensitivity = Qt::CaseInsensitive;
}

QSvgStyleSelector::~QSvgStyleSelector()
{
}

QString QSvgStyleSelector::nodeToName(QSvgNode *node) const
{
    return node->typeName();
}

QSvgNode *QSvgStyleSelector::svgNode(NodePtr node) const
{
    return (QSvgNode*)node.ptr;
}

QSvgStructureNode *QSvgStyleSelector::nodeToStructure(QSvgNode *n) const
{
    if (n &&
        (n->type() == QSvgNode::Doc ||
         n->type() == QSvgNode::Group ||
         n->type() == QSvgNode::Defs ||
         n->type() == QSvgNode::Switch)) {
        return (QSvgStructureNode*)n;
    }
    return 0;
}

QSvgStructureNode *QSvgStyleSelector::svgStructure(NodePtr node) const
{
    QSvgNode *n = svgNode(node);
    QSvgStructureNode *st = nodeToStructure(n);
    return st;
}

bool QSvgStyleSelector::nodeNameEquals(NodePtr node, const QString &nodeName) const
{
    QSvgNode *n = svgNode(node);
    if (!n)
        return false;
    QString name = nodeToName(n);
    return QString::compare(name, nodeName, Qt::CaseInsensitive) == 0;
}

QString QSvgStyleSelector::attributeValue(NodePtr node, const QCss::AttributeSelector &asel) const
{
    const QString &name = asel.name;
    QSvgNode *n = svgNode(node);
    if ((!n->nodeId().isEmpty() && (name == QLatin1String("id") ||
                                    name == QLatin1String("xml:id"))))
        return n->nodeId();
    if (!n->xmlClass().isEmpty() && name == QLatin1String("class"))
        return n->xmlClass();
    return QString();
}

bool QSvgStyleSelector::hasAttributes(NodePtr node) const
{
    QSvgNode *n = svgNode(node);
    return (n &&
            (!n->nodeId().isEmpty() || !n->xmlClass().isEmpty()));
}

QStringList QSvgStyleSelector::nodeIds(NodePtr node) const
{
    QSvgNode *n = svgNode(node);
    QString nid;
    if (n)
        nid = n->nodeId();
    QStringList lst; lst.append(nid);
    return lst;
}

QStringList QSvgStyleSelector::nodeNames(NodePtr node) const
{
    QSvgNode *n = svgNode(node);
    if (n)
        return QStringList(nodeToName(n));
    return QStringList();
}

bool QSvgStyleSelector::isNullNode(NodePtr node) const
{
    return !node.ptr;
}

QCss::StyleSelector::NodePtr QSvgStyleSelector::parentNode(NodePtr node) const
{
    QSvgNode *n = svgNode(node);
    NodePtr newNode;
    newNode.ptr = 0;
    newNode.id = 0;
    if (n) {
        QSvgNode *svgParent = n->parent();
        if (svgParent) {
            newNode.ptr = svgParent;
        }
    }
    return newNode;
}

QCss::StyleSelector::NodePtr QSvgStyleSelector::previousSiblingNode(NodePtr node) const
{
    NodePtr newNode;
    newNode.ptr = 0;
    newNode.id = 0;

    QSvgNode *n = svgNode(node);
    if (!n)
        return newNode;
    QSvgStructureNode *svgParent = nodeToStructure(n->parent());

    if (svgParent) {
        newNode.ptr = svgParent->previousSiblingNode(n);
    }
    return newNode;
}

QCss::StyleSelector::NodePtr QSvgStyleSelector::duplicateNode(NodePtr node) const
{
    NodePtr n;
    n.ptr = node.ptr;
    n.id  = node.id;
    return n;
}

void QSvgStyleSelector::freeNode(NodePtr node) const
{
    Q_UNUSED(node);
}

QT_END_NAMESPACE
