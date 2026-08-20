// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeechengine.h"

#include <QLoggingCategory>

QT_BEGIN_NAMESPACE


/*!
    \class QTextToSpeechEngine
    \inmodule QtTextToSpeech
    \brief The QTextToSpeechEngine class is the base for text-to-speech engine integrations.
    \internal

    An engine implementation must derive from QTextToSpeechEngine and implement all
    its pure virtual methods.
*/

/*!
    \fn QTextToSpeech::Capabilities QTextToSpeechEngine::capabilities() const

    Implementation of \l QTextToSpeech::engineCapabilities(). If this function is
    not implemented, then the capabilities will be read from the plugin meta data.
*/

/*!
    \fn QList<QLocale> QTextToSpeechEngine::availableLocales() const

    Implementation of \l QTextToSpeech::availableLocales().
*/

/*!
    \fn QList<QVoice> QTextToSpeechEngine::availableVoices() const

    Implementation of \l QTextToSpeech::availableVoices().
*/

/*!
    \fn void QTextToSpeechEngine::say(const QString &text)

    Implementation of \l {QTextToSpeech::say()}{QTextToSpeech::say}(\a text).
*/

/*!
    \fn void QTextToSpeechEngine::stop(QTextToSpeech::BoundaryHint hint)

    Implementation of \l QTextToSpeech::stop().
*/

/*!
    \fn void QTextToSpeechEngine::pause(QTextToSpeech::BoundaryHint hint)

    Implementation of \l QTextToSpeech::pause().
*/

/*!
    \fn void QTextToSpeechEngine::resume()

    Implementation of \l QTextToSpeech::resume().
*/

/*!
    \fn void QTextToSpeechEngine::rate() const

    Implementation of \l QTextToSpeech::rate().
*/

/*!
    \fn bool QTextToSpeechEngine::setRate(double rate)

    Implementation of \l {QTextToSpeech::setRate()}{QTextToSpeech::setRate}(\a rate).

  Return \c true if the operation was successful.
*/

/*!
    \fn void QTextToSpeechEngine::pitch() const

    Implementation of \l QTextToSpeech::pitch().
*/

/*!
    \fn bool QTextToSpeechEngine::setPitch(double pitch)

    Implementation of \l {QTextToSpeech::setPitch()}{QTextToSpeech::setPitch}(\a pitch).

    Return \c true if the operation was successful.
*/

/*!
    \fn QLocale QTextToSpeechEngine::locale() const

    Implementation of QTextToSpeech::locale().
*/

/*!
    \fn bool QTextToSpeechEngine::setLocale(const QLocale &locale)

    Implementation \l {QTextToSpeech::setLocale()}{QTextToSpeech::setLocale}(\a locale).

    Return \c true if the operation was successful. In this case, the
    current voice (as returned by voice()) should also be updated to a
    new, valid value.
*/

/*!
    \fn double QTextToSpeechEngine::volume() const

    Implementation of QTextToSpeech::volume().
*/

/*!
    \fn bool QTextToSpeechEngine::setVolume(double volume)

    Implementation of \l {QTextToSpeech::setVolume()}{QTextToSpeech::setVolume}(\a volume).

    Return \c true if the operation was successful.
*/

/*!
    \fn QVoice QTextToSpeechEngine::voice() const

    Implementation of \l QTextToSpeech::voice().
*/

/*!
    \fn bool QTextToSpeechEngine::setVoice(const QVoice &voice)

    Implementation of \l {QTextToSpeech::setVoice()}{QTextToSpeech::setVoice}(\a voice).

    Return \c true if the operation was successful.
*/

/*!
    \fn QTextToSpeech::State QTextToSpeechEngine::state() const

    Implementation of QTextToSpeech::state().
*/

/*!
    \fn void QTextToSpeechEngine::stateChanged(QTextToSpeech::State state)

    Emitted when the text-to-speech engine \a state has changed.

    This signal is connected to QTextToSpeech::stateChanged() signal.
*/

/*!
    Constructs the text-to-speech engine base class with \a parent.
*/
QTextToSpeechEngine::QTextToSpeechEngine(QObject *parent):
    QObject(parent)
{
}

QTextToSpeechEngine::~QTextToSpeechEngine()
{
}

/*!
    Creates a voice for a text-to-speech engine.

    Parameters \a name, \a locale, \a gender, \a age and \a data are directly
    stored in the QVoice instance.
*/
QVoice QTextToSpeechEngine::createVoice(const QString &name, const QLocale &locale,
                                        QVoice::Gender gender, QVoice::Age age,
                                        const QVariant &data)
{
    return QVoice(name, locale, gender, age, data);
}

/*!
    Returns the engine-specific private data for the given \a voice.

*/
QVariant QTextToSpeechEngine::voiceData(const QVoice &voice)
{
    return voice.data();
}

QT_END_NAMESPACE
