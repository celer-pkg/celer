// Copyright (C) 2020 Sze Howe Koh <szehowe.koh@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "signalslotsyntaxes.h"
#include <QQuickWidget>
#include <QQuickItem>
#include <QAudioInput>

void DemoWidget::demoImplicitConversion() {
//! [implicitconversion]
    auto slider = new QSlider(this);
    auto doubleSpinBox = new QDoubleSpinBox(this);

    // OK: The compiler can convert an int into a double
    connect(slider, &QSlider::valueChanged,
            doubleSpinBox, &QDoubleSpinBox::setValue);

    // ERROR: The string table doesn't contain conversion information
    connect(slider, SIGNAL(valueChanged(int)),
            doubleSpinBox, SLOT(setValue(double)));
//! [implicitconversion]
}

void DemoWidget::demoTypeResolution() {
//! [typeresolution]
    auto audioInput = new QAudioInput(QAudioFormat(), this);
    auto widget = new QWidget(this);

    // OK
    connect(audioInput, SIGNAL(stateChanged(QAudio::State)),
            widget, SLOT(show()));

    // ERROR: The strings "State" and "QAudio::State" don't match
    using namespace QAudio;
    connect(audioInput, SIGNAL(stateChanged(State)),
            widget, SLOT(show()));

    // ...
//! [typeresolution]
}


//! [lambda]
TextSender::TextSender(QWidget *parent) : QWidget(parent) {
    lineEdit = new QLineEdit(this);
    button = new QPushButton("Send", this);

    connect(button, &QPushButton::clicked, [=] {
        emit textCompleted(lineEdit->text());
    });

    // ...
}
//! [lambda]


void DemoWidget::demoCrossLanguageConnect()
{
//! [crosslanguage]
    auto cppObj = new CppGui(this);
    auto quickWidget = new QQuickWidget(QUrl("QmlGui.qml"), this);
    auto qmlObj = quickWidget->rootObject();

    // Connect QML signal to C++ slot
    connect(qmlObj, SIGNAL(qmlSignal(QString)),
            cppObj, SLOT(cppSlot(QString)));

    // Connect C++ signal to QML slot
    connect(cppObj, SIGNAL(cppSignal(QVariant)),
            qmlObj, SLOT(qmlSlot(QVariant)));
//! [crosslanguage]
}


//! [defaultparams]
DemoWidget::DemoWidget(QWidget *parent) : QWidget(parent) {

    // OK: printNumber() will be called with a default value of 42
    connect(qApp, SIGNAL(aboutToQuit()),
            this, SLOT(printNumber()));

    // ERROR: Compiler requires compatible arguments
    connect(qApp, &QCoreApplication::aboutToQuit,
            this, &DemoWidget::printNumber);
}
//! [defaultparams]


void DemoWidget::demoOverloadConnect()
{
//! [overload]
    auto slider = new QSlider(this);
    auto lcd = new QLCDNumber(this);

    // String-based syntax
    connect(slider, SIGNAL(valueChanged(int)),
            lcd, SLOT(display(int)));

    // Functor-based syntax
    connect(slider, &QSlider::valueChanged,
            lcd, qOverload<int>(&QLCDNumber::display));
//! [overload]
}
