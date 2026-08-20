// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsgcurvestrokenode_p.h"
#include "qsgcurvestrokenode_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSGCurveStrokeNode
    \inmodule QtQuick
    \internal
*/
QSGCurveStrokeNode::QSGCurveStrokeNode()
{
    setFlag(OwnsGeometry, true);
    qsgnode_set_description(this, QLatin1StringView("curve stroke"));
    setGeometry(new QSGGeometry(attributes(), 0, 0));
    // defer updateMaterial() until later
}

void QSGCurveStrokeNode::QSGCurveStrokeNode::updateMaterial()
{
    const bool expandingInVertexShader = m_cosmetic || expandingStrokeEnabled();
    m_material.reset(new QSGCurveStrokeMaterial(this, expandingInVertexShader));
    setMaterial(m_material.data());
}

// Take the start, control and end point of a curve and return the points A, B, C
// representing the curve as Q(s) = A*s*s + B*s + C
std::array<QVector2D, 3> QSGCurveStrokeNode::curveABC(const std::array<QVector2D, 3> &p)
{
    QVector2D a = p[0] - 2*p[1] + p[2];
    QVector2D b = 2*p[1] - 2*p[0];
    QVector2D c = p[0];

    return {a, b, c};
}

/*!
    Append a triangle with \a vtx corners within which the fragment shader will
    draw the visible part of a quadratic curve from ctl[0] to ctl[2] with
    control point ctl[1] (AKA a quadratic Bézier curve with 3 control points).
    The \a normal vectors are used in the vertex shader to expand the triangle
    according to its stroke width: therefore, it's ok for the triangle to be
    degenerate, and get expanded to size in the vertex shader. Normals are
    usually unit vectors, but it's also ok for some to have larger magnitudes,
    to handle the case when miter corners need to be extended proportionally
    farther as stroke width increases.
*/
void QSGCurveStrokeNode::appendTriangle(const std::array<QVector2D, 3> &vtx,
                                        const std::array<QVector2D, 3> &ctl,
                                        const std::array<QVector2D, 3> &normal,
                                        const std::array<float, 3> &extrusions)
{
    auto abc = curveABC(ctl);

    int currentVertex = m_uncookedVertexes.count();

    for (int i = 0; i < 3; ++i) {
        m_uncookedVertexes.append( { vtx[i].x(), vtx[i].y(),
                                   abc[0].x(), abc[0].y(), abc[1].x(), abc[1].y(), abc[2].x(), abc[2].y(),
                                   normal[i].x(), normal[i].y(), extrusions[i] } );
    }
    m_uncookedIndexes << currentVertex << currentVertex + 1 << currentVertex + 2;
}

/*!
    Append a triangle with \a vtx corners within which the fragment shader will
    draw the visible part of a line from ctl[0] to ctl[2].
    The \a normal vectors are used in the vertex shader to expand the triangle
    according to its stroke width: therefore, it's ok for the triangle to be
    degenerate, and get expanded to size in the vertex shader. Normals are
    usually unit vectors, but it's also ok for some to have larger magnitudes,
    to handle the case when miter corners need to be extended proportionally
    farther as stroke width increases.
*/
void QSGCurveStrokeNode::appendTriangle(const std::array<QVector2D, 3> &vtx,
                                        const std::array<QVector2D, 2> &ctl,
                                        const std::array<QVector2D, 3> &normal,
                                        const std::array<float, 3> &extrusions)
{
    // We could reduce this to a linear equation by setting A to (0,0).
    // However, then we cannot use the cubic solution and need an additional
    // code path in the shader. The following formulation looks more complicated
    // but allows to always use the cubic solution.
    auto A = ctl[1] - ctl[0];
    auto B = QVector2D(0., 0.);
    auto C = ctl[0];

    int currentVertex = m_uncookedVertexes.count();

    for (int i = 0; i < 3; ++i) {
        m_uncookedVertexes.append( { vtx[i].x(), vtx[i].y(),
                                    A.x(), A.y(), B.x(), B.y(), C.x(), C.y(),
                                    normal[i].x(), normal[i].y(), extrusions[i] } );
    }
    m_uncookedIndexes << currentVertex << currentVertex + 1 << currentVertex + 2;
}

void QSGCurveStrokeNode::cookGeometry()
{
    updateMaterial(); // by now, setCosmeticStroke has been called if necessary
    QSGGeometry *g = geometry();
    if (g->indexType() != QSGGeometry::UnsignedIntType) {
        g = new QSGGeometry(attributes(),
                            m_uncookedVertexes.size(),
                            m_uncookedIndexes.size(),
                            QSGGeometry::UnsignedIntType);
        setGeometry(g);
    } else {
        g->allocate(m_uncookedVertexes.size(), m_uncookedIndexes.size());
    }

    g->setDrawingMode(QSGGeometry::DrawTriangles);
    memcpy(g->vertexData(),
           m_uncookedVertexes.constData(),
           g->vertexCount() * g->sizeOfVertex());
    memcpy(g->indexData(),
           m_uncookedIndexes.constData(),
           g->indexCount() * g->sizeOfIndex());

    m_uncookedIndexes.clear();
    m_uncookedIndexes.squeeze();
    m_uncookedVertexes.clear();
    m_uncookedVertexes.squeeze();
}

const QSGGeometry::AttributeSet &QSGCurveStrokeNode::attributes()
{
    static QSGGeometry::Attribute data[] = {
        QSGGeometry::Attribute::createWithAttributeType(0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute), //vertexCoord
        QSGGeometry::Attribute::createWithAttributeType(1, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), //A
        QSGGeometry::Attribute::createWithAttributeType(2, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), //B
        QSGGeometry::Attribute::createWithAttributeType(3, 2, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), //C
        QSGGeometry::Attribute::createWithAttributeType(4, 3, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute), //normalExt
    };
    static QSGGeometry::AttributeSet attrs = { 5, sizeof(StrokeVertex), data };
    return attrs;
}

// TODO remove when we consider the expanding-stroke shader to be stable for full-time use
bool QSGCurveStrokeNode::expandingStrokeEnabled()
{
    static const bool ret = qEnvironmentVariableIntValue("QT_QUICKSHAPES_STROKE_EXPANDING");
    return ret;
}

QT_END_NAMESPACE
