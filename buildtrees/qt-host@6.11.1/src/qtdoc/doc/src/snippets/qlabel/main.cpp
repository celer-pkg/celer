// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>

class Updater : public QObject
{
    Q_OBJECT

public:
    Updater(QWidget *widget);

public slots:
    void adjustSize();

private:
    QWidget *widget;
};

Updater::Updater(QWidget *widget)
    : widget(widget)
{
}

void Updater::adjustSize()
{
    widget->adjustSize();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel *label = new QLabel("My label");
    QLineEdit *editor = new QLineEdit("New text");
    QWidget window;
    //Updater updater(&label);
    QObject::connect(editor, SIGNAL(textChanged(QString)),
                     label, SLOT(setText(QString)));
    //QObject::connect(editor, SIGNAL(textChanged(QString)),
    //                 &updater, SLOT(adjustSize()));
    //editor.show();
    //label.show();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(editor);
    window.setLayout(layout);
    window.show();
    return app.exec();
}

#include "main.moc"
