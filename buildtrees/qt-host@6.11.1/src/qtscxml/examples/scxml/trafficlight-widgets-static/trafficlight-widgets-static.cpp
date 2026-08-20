// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "statemachine.h"
#include "../trafficlight-common/trafficlight.h"

#include <QtWidgets/qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    TrafficLightStateMachine machine;
    TrafficLight widget(&machine);
    widget.show();
    machine.start();

    return app.exec();
}
