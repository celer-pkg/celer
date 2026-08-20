// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
/*!
    \class QDefaultMaskGenerator

    \inmodule QtWebSockets

    \brief The QDefaultMaskGenerator class provides the default mask generator for QtWebSockets.

    The WebSockets specification as outlined in \l {RFC 6455}
    requires that all communication from client to server must be masked. This is to prevent
    malicious scripts to attack bad behaving proxies.
    For more information about the importance of good masking,
    see \l {"Talking to Yourself for Fun and Profit" by Lin-Shung Huang et al}.
    The default mask generator uses the reasonably secure QRandomGenerator::global()->generate() function.
    The best measure against attacks mentioned in the document above,
    is to use QWebSocket over a secure connection (\e wss://).
    In general, always be careful to not have 3rd party script access to
    a QWebSocket in your application.

    \internal
*/

#include "qdefaultmaskgenerator_p.h"
#include <QRandomGenerator>

QT_BEGIN_NAMESPACE

/*!
    Constructs a new QDefaultMaskGenerator with the given \a parent.

    \internal
*/
QDefaultMaskGenerator::QDefaultMaskGenerator(QObject *parent) :
    QMaskGenerator(parent)
{
}

/*!
    Destroys the QDefaultMaskGenerator object.

    \internal
*/
QDefaultMaskGenerator::~QDefaultMaskGenerator()
{
}

/*!
    \internal
*/
bool QDefaultMaskGenerator::seed() noexcept
{
    return true;
}

/*!
    Generates a new random mask using the insecure QRandomGenerator::global()->generate() method.

    \internal
*/
quint32 QDefaultMaskGenerator::nextMask() noexcept
{
    quint32 value = QRandomGenerator::global()->generate();
    while (Q_UNLIKELY(value == 0)) {
        // a mask of zero has a special meaning
        value = QRandomGenerator::global()->generate();
    }
    return value;
}

QT_END_NAMESPACE
