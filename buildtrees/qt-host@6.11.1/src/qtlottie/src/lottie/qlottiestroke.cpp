// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiestroke_p.h"

#include <QLoggingCategory>

#include "qlottieconstants_p.h"

QT_BEGIN_NAMESPACE

QLottieStroke::QLottieStroke(const QLottieStroke &other)
    : QLottieShape(other)
{
    m_opacity = other.m_opacity;
    m_width = other.m_width;
    m_color = other.m_color;
    m_capStyle = other.m_capStyle;
    m_joinStyle = other.m_joinStyle;
    m_miterLimit = other.m_miterLimit;
    m_dashOffset = other.m_dashOffset;
    m_dashLength = other.m_dashLength;
    m_dashGap = other.m_dashGap;
    m_isDashed = other.m_isDashed;
}

QLottieStroke::QLottieStroke(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieStroke::clone() const
{
    return new QLottieStroke(*this);
}

void QLottieStroke::updateProperties(int frame)
{
    m_opacity.update(frame);
    m_width.update(frame);
    m_color.update(frame);
    if (m_isDashed) {
        m_dashOffset.update(frame);
        m_dashLength.update(frame);
        m_dashGap.update(frame);
    }
}

void QLottieStroke::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

int QLottieStroke::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieStroke::QLottieStroke()" << m_name;

    if (!checkRequiredKeys(definition, "Stroke"_L1, { "c"_L1, "o"_L1, "w"_L1 }, m_name))
        return -1;

    int lineCap = definition.value("lc"_L1).toVariant().toInt();
    switch (lineCap) {
    case 1:
        m_capStyle = Qt::FlatCap;
        break;
    case 2:
        m_capStyle = Qt::RoundCap;
        break;
    case 3:
        m_capStyle = Qt::SquareCap;
        break;
    default:
        qCDebug(lcLottieQtLottieParser) << "Unknown line cap style in QLottieStroke";
    }

    int lineJoin = definition.value("lj"_L1).toVariant().toInt();
    switch (lineJoin) {
    case 1:
        m_joinStyle = Qt::MiterJoin;
        m_miterLimit = definition.value("ml"_L1).toVariant().toReal();
        break;
    case 2:
        m_joinStyle = Qt::RoundJoin;
        break;
    case 3:
        m_joinStyle = Qt::BevelJoin;
        break;
    default:
        qCDebug(lcLottieQtLottieParser) << "Unknown line join style in QLottieStroke";
    }

    QJsonObject opacity = definition.value("o"_L1).toObject();
    opacity = resolveExpression(opacity);
    m_opacity.construct(opacity);

    QJsonObject width = definition.value("w"_L1).toObject();
    width = resolveExpression(width);
    m_width.construct(width);

    QJsonObject color = definition.value("c"_L1).toObject();
    color = resolveExpression(color);
    m_color.construct(color);

    QJsonArray dashes = definition.value("d"_L1).toArray();
    if (dashes.size()) {
        auto it = dashes.cend();
        while (it != dashes.cbegin()) {
            --it;
            QJsonObject dashSpec = it->toObject();
            QJsonObject val = resolveExpression(dashSpec.value("v"_L1).toObject());
            QString n = dashSpec.value("n"_L1).toString();
            if (n == "o"_L1)
                m_dashOffset.construct(val);
            else if (n == "g"_L1)
                m_dashGap.construct(val);
            else if (n == "d"_L1)
                m_dashLength.construct(val);
        }
        m_isDashed = true;
    }

    return 0;
}

QPen QLottieStroke::pen() const
{
    qreal width = m_width.value();
    if (qFuzzyIsNull(width))
        return QPen(Qt::NoPen);
    QPen pen;
    pen.setColor(getColor());
    pen.setWidthF(width);
    pen.setCapStyle(m_capStyle);
    pen.setJoinStyle(m_joinStyle);
    pen.setMiterLimit(m_miterLimit);
    if (m_isDashed) {
        pen.setDashOffset(m_dashOffset.value() / width);
        pen.setDashPattern({m_dashLength.value() / width, m_dashGap.value() / width});
    }
    return pen;
}

QColor QLottieStroke::getColor() const
{
    QVector4D cVec = m_color.value();
    QColor color;
    qreal r = static_cast<qreal>(cVec.x());
    qreal g = static_cast<qreal>(cVec.y());
    qreal b = static_cast<qreal>(cVec.z());
    qreal a = static_cast<qreal>(cVec.w());
    color.setRgbF(r, g, b, a);
    return color;
}

qreal QLottieStroke::opacity() const
{
    return m_opacity.value();
}

QT_END_NAMESPACE
