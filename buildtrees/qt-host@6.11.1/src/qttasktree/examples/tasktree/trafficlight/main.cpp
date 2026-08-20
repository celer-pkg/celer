// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "glueinterface.h"
#include "recipe.h"
#include "trafficlight.h"

#include <QApplication>
#include <QTaskTree>

using namespace QtTaskTree;

//! [0]
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    GlueInterface iface;
    QTaskTree taskTree({recipe(iface)});
    TrafficLight widget(iface);

    widget.show();
    taskTree.start();

    return app.exec();
}
//! [0]
