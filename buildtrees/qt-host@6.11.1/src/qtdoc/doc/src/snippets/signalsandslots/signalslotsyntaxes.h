// Copyright (C) 2014 Sze Howe Koh <szehowe.koh@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

class DemoWidget : public QWidget {
    Q_OBJECT

public:
    DemoWidget(QWidget *parent = nullptr);

    void demoImplicitConversion();
    void demoTypeResolution();
    void demoCrossLanguageConnect();
    void demoOverloadConnect();

//! [defaultparams]
public slots:
    void printNumber(int number = 42) {
        qDebug() << "Lucky number" << number;
    }
//! [defaultparams]
};


//! [lambda]
class TextSender : public QWidget {
    Q_OBJECT

    QLineEdit *lineEdit;
    QPushButton *button;

signals:
    void textCompleted(const QString& text) const;

public:
    TextSender(QWidget *parent = nullptr);
};
//! [lambda]


//! [crosslanguage]
class CppGui : public QWidget {
    Q_OBJECT

    QPushButton *button;

signals:
    void cppSignal(const QVariant& sentMsg) const;

public slots:
    void cppSlot(const QString& receivedMsg) const {
        qDebug() << "C++ received:" << receivedMsg;
    }

public:
    CppGui(QWidget *parent = nullptr) : QWidget(parent) {
        button = new QPushButton("Click Me!", this);
        connect(button, &QPushButton::clicked, [=] {
            emit cppSignal("Hello from C++!");
        });
    }
};
//! [crosslanguage]
