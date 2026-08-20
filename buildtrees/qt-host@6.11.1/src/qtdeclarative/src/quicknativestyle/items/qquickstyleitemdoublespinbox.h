// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKSTYLEITEMDOUBLESPINBOX_H
#define QQUICKSTYLEITEMDOUBLESPINBOX_H

#include "qquickstyleitem.h"
#include <QtQuickTemplates2/private/qquickdoublespinbox_p.h>

QT_BEGIN_NAMESPACE

class QQuickStyleItemDoubleSpinBox : public QQuickStyleItem
{
    Q_OBJECT

    Q_PROPERTY(SubControl subControl MEMBER m_subControl)

    QML_NAMED_ELEMENT(DoubleSpinBox)

public:
    enum SubControl {
        Frame = 1,
        Up,
        Down,
    };
    Q_ENUM(SubControl)

    QFont styleFont(QQuickItem *control) const override;

protected:
    void connectToControl() const override;
    void paintEvent(QPainter *painter) const override;
    StyleItemGeometry calculateGeometry() override;

private:
    void initStyleOption(QStyleOptionSpinBox &styleOption) const;

private:
    SubControl m_subControl = Frame;
};

QT_END_NAMESPACE

#endif // QQUICKSTYLEITEMDOUBLESPINBOX_H
