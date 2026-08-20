// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef BUTTONWRAPPER_H
#define BUTTONWRAPPER_H

#include <QObject>
class QPushButton;

class ButtonWrapper : public QObject
{
    Q_OBJECT
public:
    ButtonWrapper(QPushButton *button);

public Q_SLOTS:
    void setEnabled(int state);

private:
    QPushButton *m_button;
};

#endif // BUTTONWRAPPER_H
