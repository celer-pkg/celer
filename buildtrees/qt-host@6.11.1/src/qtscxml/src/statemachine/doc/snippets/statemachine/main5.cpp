// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtStateMachine>

int main(int argv, char **args)
{
    QApplication app(argv, args);
    QWidget *button;

  {
//![0]
    QStateMachine machine;
    machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

    QState *s1 = new QState();
    s1->assignProperty(object, "fooBar", 1.0);
    machine.addState(s1);
    machine.setInitialState(s1);

    QState *s2 = new QState();
    machine.addState(s2);
//![0]
  }

  {

//![2]
    QStateMachine machine;
    machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);

    QState *s1 = new QState();
    s1->assignProperty(object, "fooBar", 1.0);
    machine.addState(s1);
    machine.setInitialState(s1);

    QState *s2 = new QState(s1);
    s2->assignProperty(object, "fooBar", 2.0);
    s1->setInitialState(s2);

    QState *s3 = new QState(s1);
//![2]

  }

  {
//![3]
    QState *s1 = new QState();
    QState *s2 = new QState();

    s1->assignProperty(button, "geometry", QRectF(0, 0, 50, 50));
    s2->assignProperty(button, "geometry", QRectF(0, 0, 100, 100));

    s1->addTransition(button, &QPushButton::clicked, s2);
//![3]

  }

  {
//![4]
    QState *s1 = new QState();
    QState *s2 = new QState();

    s1->assignProperty(button, "geometry", QRectF(0, 0, 50, 50));
    s2->assignProperty(button, "geometry", QRectF(0, 0, 100, 100));

    QSignalTransition *transition = s1->addTransition(button, &QPushButton::clicked, s2);
    transition->addAnimation(new QPropertyAnimation(button, "geometry"));
//![4]

  }

  {
    QMainWindow *mainWindow = 0;

//![5]
    QMessageBox *messageBox = new QMessageBox(mainWindow);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->setText("Button geometry has been set!");
    messageBox->setIcon(QMessageBox::Information);

    QState *s1 = new QState();

    QState *s2 = new QState();
    s2->assignProperty(button, "geometry", QRectF(0, 0, 50, 50));
    connect(s2, &QState::entered, messageBox, SLOT(exec()));

    s1->addTransition(button, &QPushButton::clicked, s2);
//![5]
  }

  {
    QMainWindow *mainWindow = 0;

//![6]
    QMessageBox *messageBox = new QMessageBox(mainWindow);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->setText("Button geometry has been set!");
    messageBox->setIcon(QMessageBox::Information);

    QState *s1 = new QState();

    QState *s2 = new QState();
    s2->assignProperty(button, "geometry", QRectF(0, 0, 50, 50));

    QState *s3 = new QState();
    connect(s3, &QState::entered, messageBox, SLOT(exec()));

    s1->addTransition(button, &QPushButton::clicked, s2);
    s2->addTransition(s2, &QState::propertiesAssigned, s3);
//![6]

  }

  {

//![7]
    QState *s1 = new QState();
    QState *s2 = new QState();

    s2->assignProperty(object, "fooBar", 2.0);
    s1->addTransition(s2);

    QStateMachine machine;
    machine.setInitialState(s1);
    machine.addDefaultAnimation(new QPropertyAnimation(object, "fooBar"));
//![7]

  }



    return app.exec();
}

