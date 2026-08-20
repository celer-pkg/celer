// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtStateMachine>

int main(int argv, char **args)
{
    QApplication app(argv, args);

//![0]
    QState *s1 = new QState(QState::ParallelStates);
    // s11 and s12 will be entered in parallel
    QState *s11 = new QState(s1);
    QState *s12 = new QState(s1);
//![0]

//![1]
  s1->addTransition(s1, &QState::finished, s2);
//![1]

    return app.exec();
}

