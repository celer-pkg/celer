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

#ifndef COLORGRADIENT_P_H
#define COLORGRADIENT_P_H

#include <private/datavisualizationglobal_p.h>
#include <QtGui/QColor>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class ColorGradientStop : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    ColorGradientStop(QObject *parent = 0);

    qreal position() const;
    void setPosition(qreal position);

    QColor color() const;
    void setColor(const QColor &color);

Q_SIGNALS:
    void positionChanged(qreal position);
    void colorChanged(const QColor &color);

private:
    void updateGradient();

private:
    qreal m_position;
    QColor m_color;
};

class ColorGradient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<ColorGradientStop> stops READ stops)
    Q_CLASSINFO("DefaultProperty", "stops")

    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    ColorGradient(QObject *parent = 0);
    ~ColorGradient();

    QQmlListProperty<ColorGradientStop> stops();

    void doUpdate();
    QList<ColorGradientStop *> m_stops;

Q_SIGNALS:
    void updated();
};

QT_END_NAMESPACE

#endif
