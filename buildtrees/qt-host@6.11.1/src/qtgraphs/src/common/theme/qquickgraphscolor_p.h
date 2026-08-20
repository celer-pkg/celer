// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QQUICKGRAPHSCOLOR_P_H
#define QQUICKGRAPHSCOLOR_P_H

#include <QtGui/QColor>
#include <QtQml/qqml.h>
#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE
enum class GradientType {
    Base,
    Single,
    Multi,
};

class QQuickGraphsColor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    QML_NAMED_ELEMENT(Color)

public:
    QQuickGraphsColor(QObject *parent = 0);
    ~QQuickGraphsColor() override;

    void setColor(QColor color);
    QColor color() const;

Q_SIGNALS:
    void colorChanged(QColor color);

private:
    QColor m_color;
};

QT_END_NAMESPACE

#endif
