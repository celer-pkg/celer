// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "../trafficlight-common/trafficlight.h"

#include <QtWidgets/qapplication.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qtextstream.h>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QScxmlStateMachine *machine = QScxmlStateMachine::fromFile(u":statemachine.scxml"_s);

    if (!machine->parseErrors().isEmpty()) {
        QTextStream errs(stderr, QIODevice::WriteOnly);
        const auto errors = machine->parseErrors();
        for (const QScxmlError &error : errors) {
            errs << error.toString();
        }
        return -1;
    }

    TrafficLight widget(machine);
    widget.show();
    machine->setParent(&widget);
    machine->start();

    return app.exec();
}
