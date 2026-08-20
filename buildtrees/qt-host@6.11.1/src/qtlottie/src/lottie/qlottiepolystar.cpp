// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiepolystar_p.h"

#include <QJsonObject>
#include <QRectF>
#include <QString>

#include "qlottietrimpath_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePolyStar::QLottiePolyStar(const QLottiePolyStar &other)
    : QLottieShape(other)
{
    m_position = other.m_position;
    m_pointCount = other.m_pointCount;
    m_outerRadius = other.m_outerRadius;
    m_innerRadius = other.m_innerRadius;
    m_startAngle = other.m_startAngle;
    m_polygonMode = other.m_polygonMode;
}

QLottiePolyStar::QLottiePolyStar(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottiePolyStar::clone() const
{
    return new QLottiePolyStar(*this);
}

int QLottiePolyStar::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottiePolyStar::parse():" << m_name;

    if (!checkRequiredKeys(definition, "Polystar"_L1, {"p"_L1, "or"_L1, "os"_L1, "pt"_L1, "r"_L1}, m_name))
        return -1;

    QJsonObject position = definition.value("p"_L1).toObject();
    position = resolveExpression(position);
    m_position.construct(position);

    QJsonObject outerRadius = definition.value("or"_L1).toObject();
    outerRadius = resolveExpression(outerRadius);
    m_outerRadius.construct(outerRadius);

    if (definition.contains("ir"_L1)) {
        QJsonObject innerRadius = definition.value("ir"_L1).toObject();
        innerRadius = resolveExpression(innerRadius);
        m_innerRadius.construct(innerRadius);
    }

    QJsonObject startAngle = definition.value("r"_L1).toObject();
    startAngle = resolveExpression(startAngle);
    m_startAngle.construct(startAngle);

    QJsonObject pointCount = definition.value("pt"_L1).toObject();
    pointCount = resolveExpression(pointCount);
    m_pointCount.construct(pointCount);

    m_polygonMode = (definition.value("sy"_L1).toInt() == 2);

    m_direction = definition.value("d"_L1).toInt();

    return 0;
}

bool QLottiePolyStar::acceptsTrim() const
{
    return true;
}

void QLottiePolyStar::updateProperties(int frame)
{
    m_position.update(frame);
    m_outerRadius.update(frame);
    m_innerRadius.update(frame);
    m_startAngle.update(frame);
    m_pointCount.update(frame);

    m_path.clear();

    const int numPoints = m_pointCount.value();
    if (numPoints < 1)
        return;
    const qreal angleStep = -360.0 / numPoints;
    const qreal halfAngleStep = angleStep / 2;
    qreal angle = 90 - m_startAngle.value();

    const QPointF pos = m_position.value();
    QLineF outLine(pos, QPointF(pos.x(), pos.y() - m_outerRadius.value()));
    QLineF inLine(pos, QPointF(pos.x(), pos.y() - m_innerRadius.value()));

    outLine.setAngle(angle);
    const QPointF startPt = outLine.p2();
    m_path.moveTo(startPt);
    for (int i = 0; i < numPoints; i++) {
        if (!m_polygonMode) {
            inLine.setAngle(angle + halfAngleStep);
            m_path.lineTo(inLine.p2());
        }
        angle += angleStep;
        outLine.setAngle(angle);
        m_path.lineTo(outLine.p2());
    }
    // Fix potential accuracy errors, ensuring path is closed:
    m_path.setElementPositionAt(m_path.elementCount() - 1, startPt.x(), startPt.y());

    if (hasReversedDirection())
        m_path = m_path.toReversed();
}

void QLottiePolyStar::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

QPointF QLottiePolyStar::position() const
{
    return m_position.value();
}

qreal QLottiePolyStar::outerRadius() const
{
    return m_outerRadius.value();
}

qreal QLottiePolyStar::innerRadius() const
{
    return m_innerRadius.value();
}

qreal QLottiePolyStar::startAngle() const
{
    return m_startAngle.value();
}

int QLottiePolyStar::pointCount() const
{
    return m_pointCount.value();
}

bool QLottiePolyStar::isPolygonModeEnabled() const
{
    return m_polygonMode;
}

QT_END_NAMESPACE
