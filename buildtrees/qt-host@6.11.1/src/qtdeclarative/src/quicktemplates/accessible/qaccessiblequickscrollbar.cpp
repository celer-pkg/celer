// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qaccessiblequickscrollbar_p.h"
#include "qquickscrollbar_p.h"

QT_BEGIN_NAMESPACE

QAccessibleQuickScrollBar::QAccessibleQuickScrollBar(QQuickScrollBar *scrollBar)
    : QAccessibleQuickControl(scrollBar)
{
}

QList<QAccessible::Attribute> QAccessibleQuickScrollBar::attributeKeys() const
{
    QList<QAccessible::Attribute> keys = QAccessibleQuickControl::attributeKeys();
    keys.append(QAccessible::Attribute::Orientation);

    return keys;
}

QVariant QAccessibleQuickScrollBar::attributeValue(QAccessible::Attribute key) const
{
    if (key == QAccessible::Attribute::Orientation)
        return QVariant::fromValue(scrollBar()->orientation());

    return QAccessibleQuickControl::attributeValue(key);
}

QVariant QAccessibleQuickScrollBar::currentValue() const
{
    return normalizeValue(scrollBar()->position());
}

void QAccessibleQuickScrollBar::setCurrentValue(const QVariant &value)
{
    scrollBar()->setPosition(denormalizeValue(value.toReal()));
}

QVariant QAccessibleQuickScrollBar::maximumValue() const
{
    return normalizeValue(1.0 - scrollBar()->size());
}

QVariant QAccessibleQuickScrollBar::minimumValue() const
{
    return normalizeValue(0.0);
}

QVariant QAccessibleQuickScrollBar::minimumStepSize() const
{
    return normalizeValue(scrollBar()->stepSize());
}

QQuickScrollBar *QAccessibleQuickScrollBar::scrollBar() const
{
    return static_cast<QQuickScrollBar *>(object());
}

qreal QAccessibleQuickScrollBar::maximumPositionValue() const
{
    return 1.0 - scrollBar()->size();
}

qreal QAccessibleQuickScrollBar::normalizeValue(qreal value) const
{
    const qreal maxPositionValue = maximumPositionValue();
    if (maxPositionValue > 0.0)
        return value / maxPositionValue * 100.0;
    return 0;
}

qreal QAccessibleQuickScrollBar::denormalizeValue(qreal value) const
{
    return value / 100.0 * maximumPositionValue();
}

QT_END_NAMESPACE
