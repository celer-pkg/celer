// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgvisitor_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

static const char *nodeTypeStrings[] = {
    "DOC",
    "G",
    "DEFS",
    "SWITCH",
    "ANIMATION",
    "ARC",
    "CIRCLE",
    "ELLIPSE",
    "IMAGE",
    "LINE",
    "PATH",
    "POLYGON",
    "POLYLINE",
    "RECT",
    "TEXT",
    "TEXTAREA",
    "TSPAN",
    "USE",
    "VIDEO"
};

// TODO: something like this is needed in several places. Make a common version.
static const char *typeName(const QSvgNode *node)
{
    constexpr int typeNameCount = sizeof(nodeTypeStrings) / sizeof(const char *);
    if (node->type() < typeNameCount)
        return nodeTypeStrings[node->type()];
    return "UNKNOWN";
}

class SvgDebugVisitor : public QSvgVisitor
{
public:
    SvgDebugVisitor(QDebug &stream) : debug(stream) {}
    void write(const QSvgDocument *doc);

protected:
    void visitNode(const QSvgNode *) override;
    bool visitStructureNodeStart(const QSvgStructureNode *node) override;
    void visitStructureNodeEnd(const QSvgStructureNode *) override;
    void visitAnimateNode(const QSvgAnimateNode *node) override;
    void visitEllipseNode(const QSvgEllipse *node) override;
    void visitImageNode(const QSvgImage *node) override;
    void visitLineNode(const QSvgLine *node) override;
    void visitPathNode(const QSvgPath *node) override;
    void visitPolygonNode(const QSvgPolygon *node) override;
    void visitPolylineNode(const QSvgPolyline *node) override;
    void visitRectNode(const QSvgRect *node) override;
    void visitTextNode(const QSvgText *node) override;
    void visitUseNode(const QSvgUse *node) override;
    void visitVideoNode(const QSvgVideo *node) override;

private:
    const char *indent() { m_indent.fill(' ', m_indentLevel * 2);  return m_indent.constData();}
    void handleBaseNode(const QSvgNode *node);
    QDebug &debug;
    int m_indentLevel = 0;
    QByteArray m_indent;
    int nodeCounter = 0;
};

void SvgDebugVisitor::handleBaseNode(const QSvgNode *node)
{
    debug << indent() << typeName(node) << "node, ID:" << node->nodeId();
    nodeCounter++;
}

void SvgDebugVisitor::visitNode(const QSvgNode *node)
{
    handleBaseNode(node);
    debug << Qt::endl;
}

bool SvgDebugVisitor::visitStructureNodeStart(const QSvgStructureNode *node)
{
    debug << indent() << "START node" << node->nodeId() << "type" << typeName(node) << node->type() << Qt::endl;
    m_indentLevel++;
    return true;
}

void SvgDebugVisitor::visitStructureNodeEnd(const QSvgStructureNode *node)
{
    m_indentLevel--;
    debug << indent() << "END node" << node->nodeId() << Qt::endl;
}

void SvgDebugVisitor::visitAnimateNode(const QSvgAnimateNode *node)
{
    handleBaseNode(node);
    debug << Qt::endl;
}

void SvgDebugVisitor::visitEllipseNode(const QSvgEllipse *node)
{
    handleBaseNode(node);
    debug << "rect:" << node->rect() << Qt::endl;
}

void SvgDebugVisitor::visitImageNode(const QSvgImage *node)
{
    handleBaseNode(node);
    debug << "image:" << node->image() << Qt::endl;
}

void SvgDebugVisitor::visitLineNode(const QSvgLine *node)
{
    handleBaseNode(node);
    debug << "line:" << node->line() << Qt::endl;
}

void SvgDebugVisitor::visitPathNode(const QSvgPath *node)
{
    handleBaseNode(node);
    debug << "path:" << node->path().elementCount() << "elements." << Qt::endl;
}

void SvgDebugVisitor::visitPolygonNode(const QSvgPolygon *node)
{
    handleBaseNode(node);
    debug << "polygon:" << node->polygon().size() << "elements." << Qt::endl;
}

void SvgDebugVisitor::visitPolylineNode(const QSvgPolyline *node)
{
    handleBaseNode(node);
    debug << "polygon:" << node->polygon().size() << "elements." << Qt::endl;
}

void SvgDebugVisitor::visitRectNode(const QSvgRect *node)
{
    handleBaseNode(node);
    debug << "rect:" << node->rect() << "radius:" << node->radius() << Qt::endl;
}

void SvgDebugVisitor::visitTextNode(const QSvgText *node)
{
    handleBaseNode(node);
    QString text;
    for (const auto *tspan : node->tspans()) {
        if (!tspan)
            text += QStringLiteral("\\n");
        else
            text += tspan->text();
    }
    debug << "text:" << text << Qt::endl;
}

void SvgDebugVisitor::visitUseNode(const QSvgUse *node)
{
    handleBaseNode(node);
    debug << "link ID:" << node->linkId() << Qt::endl;
}

void SvgDebugVisitor::visitVideoNode(const QSvgVideo *node)
{
    handleBaseNode(node);
    debug << Qt::endl;
}

void SvgDebugVisitor::write(const QSvgDocument *doc)
{
    debug << "SVG" << doc->size() << "viewBox" << doc->viewBox() << Qt::endl;
    traverse(doc);

    debug << "END SVG" << nodeCounter << "nodes";
}

QDebug operator<<(QDebug debug, const QSvgDocument &doc)
{
    SvgDebugVisitor visitor(debug);
    visitor.write(&doc);

    return debug;
}

QT_END_NAMESPACE
