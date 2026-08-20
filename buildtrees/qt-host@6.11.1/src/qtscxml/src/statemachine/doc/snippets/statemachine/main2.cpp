// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtStateMachine>
#include <QtWidgets>

int main(int argv, char **args)
{
    QApplication app(argv, args);

    QStateMachine machine;

//![0]
    QState *s1 = new QState();
    QState *s11 = new QState(s1);
    QState *s12 = new QState(s1);
    QState *s13 = new QState(s1);
    s1->setInitialState(s11);
    machine.addState(s1);
//![0]

//![2]
    s12->addTransition(quitButton, &QPushButton::clicked, s12);
//![2]

//![1]
    QFinalState *s2 = new QFinalState();
    s1->addTransition(quitButton, &QPushButton::clicked, s2);
    machine.addState(s2);
    machine.setInitialState(s1);

    QObject::connect(&machine, &QStateMachine::finished,
                     QCoreApplication::instance(), &QCoreApplication::quit);
//![1]

  QButton *interruptButton = new QPushButton("Interrupt Button");
  QWidget *mainWindow = new QWidget();

//![3]
    QHistoryState *s1h = new QHistoryState(s1);

    QState *s3 = new QState();
    s3->assignProperty(label, "text", "In s3");
    QMessageBox *mbox = new QMessageBox(mainWindow);
    mbox->addButton(QMessageBox::Ok);
    mbox->setText("Interrupted!");
    mbox->setIcon(QMessageBox::Information);
    QObject::connect(s3, &QState::entered, mbox, &QMessageBox::exec);
    s3->addTransition(s1h);
    machine.addState(s3);

    s1->addTransition(interruptButton, &QPushButton::clicked, s3);
//![3]

  return app.exec();
}

