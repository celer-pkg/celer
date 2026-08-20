// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [simple state machine]
QPushButton button;

QStateMachine machine;
QState *s1 = new QState();
s1->assignProperty(&button, "text", "Click me");

QFinalState *s2 = new QFinalState();
s1->addTransition(&button, &QPushButton::clicked, s2);

machine.addState(s1);
machine.addState(s2);
machine.setInitialState(s1);
machine.start();
//! [simple state machine]
