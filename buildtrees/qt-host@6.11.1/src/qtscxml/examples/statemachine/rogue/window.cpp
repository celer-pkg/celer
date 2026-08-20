// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "window.h"

#include <QtCore/QRandomGenerator>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtStateMachine/QFinalState>
#include <QtStateMachine/QKeyEventTransition>
#include <QtWidgets/QApplication>

#include <cmath>

#include "movementtransition.h"

//![0]
Window::Window()
{
//![0]
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(12);
    setFont(font);
//![1]
    setupMap();
    buildMachine();
}
//![1]

void Window::setStatus(const QString &status)
{
    myStatus = status;
    repaint();
}

QString Window::status() const
{
    return myStatus;
}

void Window::paintEvent(QPaintEvent * /* event */)
{
    QFontMetrics metrics(font());
    QPainter painter(this);
    int fontHeight = metrics.height();
    int fontWidth = metrics.horizontalAdvance('X');
    int yPos = fontHeight;
    int xPos;

    painter.fillRect(rect(), Qt::black);
    painter.setPen(Qt::white);

    painter.drawText(QPoint(0, yPos), status());

    for (int y = 0; y < HEIGHT; ++y) {
        yPos += fontHeight;
        xPos = 0;

        for (int x = 0; x < WIDTH; ++x) {
            if (y == pY && x == pX) {
                xPos += fontWidth;
                continue;
            }

            painter.setPen(Qt::white);

            double x1 = static_cast<double>(pX);
            double y1 = static_cast<double>(pY);
            double x2 = static_cast<double>(x);
            double y2 = static_cast<double>(y);

            if (x2 < x1) {
                x2 += 0.5;
            } else if (x2 > x1) {
                x2 -= 0.5;
            }

            if (y2 < y1) {
                y2 += 0.5;
            } else if (y2 > y1) {
                y2 -= 0.5;
            }

            double dx = x2 - x1;
            double dy = y2 - y1;

            double length = std::hypot(dx, dy);

            dx /= length;
            dy /= length;

            double xi = x1;
            double yi = y1;

            while (length > 0) {
                int cx = static_cast<int>(xi + 0.5);
                int cy = static_cast<int>(yi + 0.5);

                if (x2 == cx && y2 == cy)
                    break;

                if (!(x1 == cx && y1 == cy) && (map[cx][cy] == '#' || (length - 10) > 0)) {
                    painter.setPen(QColor(60, 60, 60));
                    break;
                }

                xi += dx;
                yi += dy;
                --length;
            }

            painter.drawText(QPoint(xPos, yPos), map[x][y]);
            xPos += fontWidth;
        }
    }
    painter.setPen(Qt::white);
    painter.drawText(QPoint(pX * fontWidth, (pY + 2) * fontHeight), QChar('@'));
}

QSize Window::sizeHint() const
{
    QFontMetrics metrics(font());

    return QSize(metrics.horizontalAdvance('X') * WIDTH, metrics.height() * (HEIGHT + 1));
}

//![2]
void Window::buildMachine()
{
    machine = new QStateMachine;

    auto inputState = new QState(machine);
    inputState->assignProperty(this, "status", "Move the rogue with 2, 4, 6, and 8");

    auto transition = new MovementTransition(this);
    inputState->addTransition(transition);
//![2]

//![3]
    auto quitState = new QState(machine);
    quitState->assignProperty(this, "status", "Really quit(y/n)?");

    auto yesTransition = new QKeyEventTransition(this, QEvent::KeyPress, Qt::Key_Y);
    yesTransition->setTargetState(new QFinalState(machine));
    quitState->addTransition(yesTransition);

    auto noTransition = new QKeyEventTransition(this, QEvent::KeyPress, Qt::Key_N);
    noTransition->setTargetState(inputState);
    quitState->addTransition(noTransition);
//![3]

//![4]
    auto quitTransition = new QKeyEventTransition(this, QEvent::KeyPress, Qt::Key_Q);
    quitTransition->setTargetState(quitState);
    inputState->addTransition(quitTransition);
//![4]

//![5]
    machine->setInitialState(inputState);

    connect(machine, &QStateMachine::finished, qApp, &QApplication::quit);

    machine->start();
}
//![5]

void Window::movePlayer(Direction direction)
{
    switch (direction) {
        case Left:
            if (map[pX - 1][pY] != '#')
                --pX;
            break;
        case Right:
            if (map[pX + 1][pY] != '#')
                ++pX;
            break;
        case Up:
            if (map[pX][pY - 1] != '#')
                --pY;
            break;
        case Down:
            if (map[pX][pY + 1] != '#')
                ++pY;
            break;
    }
    repaint();
}

void Window::setupMap()
{
    for (int x = 0; x < WIDTH; ++x)
        for (int y = 0; y < HEIGHT; ++y) {
        if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1 || QRandomGenerator::global()->bounded(40) == 0)
            map[x][y] = '#';
        else
            map[x][y] = '.';
    }
}

