// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottierect_p.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "qlottietrimpath_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieRect::QLottieRect(const QLottieRect &other)
    : QLottieShape(other)
{
    m_position = other.m_position;
    m_size = other.m_size;
    m_roundness = other.m_roundness;
}

QLottieRect::QLottieRect(QLottieBase *parent)
{
    setParent(parent);
}


QLottieBase *QLottieRect::clone() const
{
    return new QLottieRect(*this);
}

bool QLottieRect::setProperty(QLottieLiteral::PropertyType propertyType, QVariant value)
{
    switch (propertyType) {
    case QLottieLiteral::RectPosition:
        qCDebug(lcLottieQtLottieParser) << "Set position" << value.toPointF();
        m_position.setValue(value.toPointF());
        break;
    default:
        return false;
    }
    return true;
}

void QLottieRect::updateProperties(int frame)
{
    m_size.update(frame);
    m_position.update(frame);
    m_roundness.update(frame);

    // AE uses center of a shape as it's position,
    // in Qt a translation is needed
    QPointF pos = QPointF(m_position.value().x() - m_size.value().width() / 2,
                             m_position.value().y() - m_size.value().height() / 2);

    m_path = QPainterPath();
    m_path.addRoundedRect(QRectF(pos, m_size.value()),
                               m_roundness.value(), m_roundness.value());

    if (hasReversedDirection())
        m_path = m_path.toReversed();
}

void QLottieRect::render(QLottieRenderer &renderer) const
{
    renderer.render(*this);
}

bool QLottieRect::acceptsTrim() const
{
    return true;
}

int QLottieRect::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieRect::QLottieRect():" << m_name;

    if (!checkRequiredKeys(definition, "Rectangle"_L1, { "p"_L1, "s"_L1 }, m_name))
        return -1;

    QJsonObject position = definition.value("p"_L1).toObject();
    position = resolveExpression(position);
    m_position.construct(position);

    QJsonObject size = definition.value("s"_L1).toObject();
    size = resolveExpression(size);
    m_size.construct(size);

    QJsonObject roundness = definition.value("r"_L1).toObject();
    roundness = resolveExpression(roundness);
    m_roundness.construct(roundness);

    m_direction = definition.value("d"_L1).toInt();

    return 0;
}

QPointF QLottieRect::position() const
{
    return m_position.value();
}

QSizeF QLottieRect::size() const
{
    return m_size.value();
}

qreal QLottieRect::roundness() const
{
    return m_roundness.value();
}

QT_END_NAMESPACE
