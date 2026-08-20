// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeechplugin.h"

QT_BEGIN_NAMESPACE

/*!
    \class QTextToSpeechPlugin
    \inmodule QtTextToSpeech
    \brief The QTextToSpeechPlugin class is the base for all text-to-speech plug-ins.
    \internal

    A plug-in implementation should derive from QTextToSpeechPlugin and re-implement
    \l createTextToSpeechEngine().
*/

/*!
    Factory method that is triggered by a call to \l QTextToSpeech::QTextToSpeech()
    when a provider name is given in the constructor and a text-to-speech plug-in
    matching the provider name was successfully loaded.

    Value of \a parameters is currently always empty.

    If an error occurs, the method should return 0 and (optionally) give a description
    of the error in \a errorString. In this case, QTextToSpeech::state() will return
    QTextToSpeech::Error.

    If \a parent is 0, the caller takes the ownership of the returned engine instance.
*/
QTextToSpeechEngine *QTextToSpeechPlugin::createTextToSpeechEngine(
        const QVariantMap &parameters,
        QObject *parent,
        QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(parent);
    Q_UNUSED(errorString);

    return 0;
}

QT_END_NAMESPACE
