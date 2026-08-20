// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtStateMachine>

class MyTransition : public QAbstractTransition
{
    Q_OBJECT
public:
    MyTransition() {}

protected:
//![0]
    bool eventTest(QEvent *event) override
    {
        if (event->type() == QEvent::Wrapped) {
            QEvent *wrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
            if (wrappedEvent->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent *>(wrappedEvent);
                // Do your event test
            }
        }
        return false;
    }
//![0]

    void onTransition(QEvent *event) override
    {

    }
};

int main(int argv, char **args)
{
    return 0;
}
