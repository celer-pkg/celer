// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CORE_H
#define CORE_H

#include "dialog.h"
#include <QObject>

/*
    An instance of this class gets published over the WebChannel and is then accessible to HTML clients.
*/
class Core : public QObject
{
    Q_OBJECT

public:
    Core(Dialog *dialog, QObject *parent = nullptr)
        : QObject(parent), m_dialog(dialog)
    {
        connect(dialog, &Dialog::sendText, this, &Core::sendText);
    }

signals:
    /*
        This signal is emitted from the C++ side and the text displayed on the HTML client side.
    */
    void sendText(const QString &text);

public slots:

    /*
        This slot is invoked from the HTML client side and the text displayed on the server side.
    */
    void receiveText(const QString &text)
    {
        m_dialog->displayMessage(Dialog::tr("Received message: %1").arg(text));
    }

private:
    Dialog *m_dialog;
};

#endif // CORE_H
