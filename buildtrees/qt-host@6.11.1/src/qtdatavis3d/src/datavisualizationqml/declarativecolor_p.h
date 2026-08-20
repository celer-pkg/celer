// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVECOLOR_P_H
#define DECLARATIVECOLOR_P_H

#include <private/datavisualizationglobal_p.h>
#include <QtGui/QColor>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class DeclarativeColor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    QML_NAMED_ELEMENT(ThemeColor)
    QML_ADDED_IN_VERSION(1, 0)

public:
    DeclarativeColor(QObject *parent = 0);

    void setColor(const QColor &color);
    QColor color() const;

Q_SIGNALS:
    void colorChanged(const QColor &color);

private:
    QColor m_color;
};

QT_END_NAMESPACE

#endif
