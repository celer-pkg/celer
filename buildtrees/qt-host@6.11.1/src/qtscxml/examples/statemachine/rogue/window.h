// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QStateMachine);

//![0]
class Window : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus)

public:
    enum Direction { Up, Down, Left, Right };

    Window();

    void movePlayer(Direction direction);
    void setStatus(const QString &status);
    QString status() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
//![0]

//![1]
private:
    void buildMachine();
    void setupMap();

    static constexpr int WIDTH = 35;
    static constexpr int HEIGHT = 20;

    QChar map[WIDTH][HEIGHT];
    int pX = 5;
    int pY = 5;

    QStateMachine *machine;
    QString myStatus;
};
//![1]

#endif

