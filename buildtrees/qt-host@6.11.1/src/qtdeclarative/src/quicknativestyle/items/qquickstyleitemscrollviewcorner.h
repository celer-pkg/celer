// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKSTYLEITEMSCROLLVIEWCORNER_H
#define QQUICKSTYLEITEMSCROLLVIEWCORNER_H

#include "qquickstyleitem.h"
#include <QtQuickTemplates2/private/qquickscrollbar_p.h>

QT_BEGIN_NAMESPACE

class QPainter;
class QQuickStyleItemScrollViewCorner : public QQuickStyleItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ScrollViewCorner)

protected:
    void paintEvent(QPainter *painter) const override;
    StyleItemGeometry calculateGeometry() override;

private:
    void initStyleOption(QStyleOptionSlider &styleOption) const;
};

QT_END_NAMESPACE

#endif // QQUICKSTYLEITEMSCROLLVIEWCORNER_H
