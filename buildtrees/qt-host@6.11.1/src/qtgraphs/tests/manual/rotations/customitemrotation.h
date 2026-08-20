// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CUSTOMITEMROTATION_H
#define CUSTOMITEMROTATION_H

#include <QtWidgets/QWidget>
#include <QtCore/qobject.h>
#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include "customitemdatamodifier.h"

class CustomItemRotation : public QObject
{
    Q_OBJECT

public:
    CustomItemRotation(QObject *parent = nullptr);

    void initialize();
    QWidget *widget() {return m_container;};

private:
    Q3DScatterWidgetItem *graph = nullptr;
    QQuickWidget *quickWidget = nullptr;
    CustomItemDataModifier *modifier = nullptr;
    QWidget *m_container = nullptr;
};

#endif
