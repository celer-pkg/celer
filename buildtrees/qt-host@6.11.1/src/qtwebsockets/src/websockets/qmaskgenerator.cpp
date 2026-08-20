// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

/*!
    \class QMaskGenerator

    \inmodule QtWebSockets
    \since 5.3

    \brief The QMaskGenerator class provides an abstract base for custom 32-bit mask generators.

    The WebSockets specification as outlined in \l {RFC 6455}
    requires that all communication from client to server be masked. This is to prevent
    malicious scripts from attacking badly behaving proxies.
    For more information about the importance of good masking,
    see \l {"Talking to Yourself for Fun and Profit" by Lin-Shung Huang et al}.
    By default QWebSocket uses the reasonably secure QRandomGenerator::global()->generate() function.
    The best measure against attacks mentioned in the document above,
    is to use QWebSocket over a secure connection (\e wss://).
    In general, always be careful to not have 3rd party script access to
    a QWebSocket in your application.
*/

/*!
    \fn bool QMaskGenerator::seed()

    Initializes the QMaskGenerator by seeding the randomizer.
    When seed() is not called, it depends on the specific implementation of a subclass if
    a default seed is used or no seed is used at all.
    Returns \e true if seeding succeeds, otherwise false.
*/

/*!
    \fn quint32 QMaskGenerator::nextMask()

    Returns a new random 32-bit mask. The randomness depends on the RNG used to created the
    mask.
*/

#include "qmaskgenerator.h"

QT_BEGIN_NAMESPACE

/*!
    Creates a new QMaskGenerator object with the given optional QObject \a parent.
 */
QMaskGenerator::QMaskGenerator(QObject *parent) :
    QObject(parent)
{
}

/*!
    Destroys the QMaskGenerator object.
 */
QMaskGenerator::~QMaskGenerator()
{}

QT_END_NAMESPACE
