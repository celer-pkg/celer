// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieround_p.h"

#include <QJsonObject>

#include "qlottietrimpath_p.h"

QT_BEGIN_NAMESPACE

QLottieRound::QLottieRound(const QLottieRound &other)
    : QLottieShape(other)
{
    m_position = other.m_position;
    m_radius = other.m_radius;
}

QLottieRound::QLottieRound(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieRound::clone() const
{
    return new QLottieRound(*this);
}

int QLottieRound::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieRound::parse():" << m_name;

    QJsonObject position = definition.value("p"_L1).toObject();
    position = resolveExpression(position);
    m_position.construct(position);

    QJsonObject radius = definition.value("r"_L1).toObject();
    radius = resolveExpression(radius);
    m_radius.construct(radius);

    return 0;
}

void QLottieRound::updateProperties(int frame)
{
    m_position.update(frame);
    m_radius.update(frame);

    // AE uses center of a shape as it's position,
    // in Qt a translation is needed
    QPointF center = QPointF(m_position.value().x() - m_radius.value() / 2,
                             m_position.value().y() - m_radius.value() / 2);

    m_path = QPainterPath();
    m_path.arcMoveTo(QRectF(center,
                            QSizeF(m_radius.value(), m_radius.value())), 90);
    m_path.arcTo(QRectF(center,
                        QSizeF(m_radius.value(), m_radius.value())), 90, -360);

    if (hasReversedDirection())
        m_path = m_path.toReversed();
}

void QLottieRound::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

bool QLottieRound::acceptsTrim() const
{
    return true;
}

QPointF QLottieRound::position() const
{
    return m_position.value();
}

qreal QLottieRound::radius() const
{
    return m_radius.value();
}

QT_END_NAMESPACE
