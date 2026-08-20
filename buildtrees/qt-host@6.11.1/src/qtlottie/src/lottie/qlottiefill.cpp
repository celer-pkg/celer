// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiefill_p.h"

QT_BEGIN_NAMESPACE

QLottieFill::QLottieFill(const QLottieFill &other)
    : QLottieShape(other)
{
    m_color = other.m_color;
    m_opacity = other.m_opacity;
    m_fillRule = other.m_fillRule;
}

QLottieFill::QLottieFill(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieFill::clone() const
{
    return new QLottieFill(*this);
}

void QLottieFill::updateProperties(int frame)
{
    m_color.update(frame);
    m_opacity.update(frame);
}

void QLottieFill::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

int QLottieFill::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieFill::parse():" << m_name;

    if (!checkRequiredKeys(definition, "Fill"_L1, { "o"_L1, "c"_L1 }, m_name))
        return -1;

    QJsonObject color = definition.value("c"_L1).toObject();
    m_color.construct(color);

    QJsonObject opacity = definition.value("o"_L1).toObject();
    opacity = resolveExpression(opacity);
    m_opacity.construct(opacity);

    const int fillValue = definition.value("r"_L1).toInt();
    m_fillRule = (fillValue == 2) ? Qt::OddEvenFill : Qt::WindingFill;

    return 0;
}

QColor QLottieFill::color() const
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

qreal QLottieFill::opacity() const
{
    return m_opacity.value();
}

Qt::FillRule QLottieFill::fillRule() const
{
    return m_fillRule;
}

QT_END_NAMESPACE
