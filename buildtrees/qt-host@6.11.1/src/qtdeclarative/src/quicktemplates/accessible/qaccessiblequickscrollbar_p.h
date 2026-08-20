// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QACCESSIBLEQUICKSCROLLBAR_H
#define QACCESSIBLEQUICKSCROLLBAR_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qaccessiblequickcontrol_p.h"

QT_BEGIN_NAMESPACE

class QQuickScrollBar;

class QAccessibleQuickScrollBar : public QAccessibleQuickControl
{
public:
    QAccessibleQuickScrollBar(QQuickScrollBar *scrollBar);

    QList<QAccessible::Attribute> attributeKeys() const override;
    QVariant attributeValue(QAccessible::Attribute key) const override;

    QVariant currentValue() const override;
    void setCurrentValue(const QVariant &value) override;
    QVariant maximumValue() const override;
    QVariant minimumValue() const override;
    QVariant minimumStepSize() const override;

private:
    QQuickScrollBar *scrollBar() const;

    qreal maximumPositionValue() const;
    // convert QQuickScrollBar::position value to value in the range 0.0 to 100.0
    qreal normalizeValue(qreal value) const;
    // convert normalized value in range 0.0 to 100.0 to QQuickScrollBar::position value
    qreal denormalizeValue(qreal value) const;
};

QT_END_NAMESPACE

#endif // QACCESSIBLEQUICKSCROLLBAR_H
