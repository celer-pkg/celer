// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKSTYLEITEMSEARCHFIELD_H
#define QQUICKSTYLEITEMSEARCHFIELD_H

#include "qquickstyleitem.h"
#include <QtQuickTemplates2/private/qquicksearchfield_p.h>

QT_BEGIN_NAMESPACE

class QQuickStyleItemSearchField : public QQuickStyleItem
{
    Q_OBJECT

    Q_PROPERTY(SubControl subControl MEMBER m_subControl)

    QML_NAMED_ELEMENT(SearchField)

public:
    enum SubControl {
        Frame = 1,
        Search,
        Clear
    };
    Q_ENUM(SubControl)

    QFont styleFont(QQuickItem *control) const override;

protected:
    void connectToControl() const override;
    void paintEvent(QPainter *painter) const override;
    StyleItemGeometry calculateGeometry() override;

private:
    void initStyleOption(QStyleOptionSearchField &styleOption) const;

private:
    SubControl m_subControl = Frame;
};

QT_END_NAMESPACE

#endif // QQUICKSTYLEITEMSEARCHFIELD_H
