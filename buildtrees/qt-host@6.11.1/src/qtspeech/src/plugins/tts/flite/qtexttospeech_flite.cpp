// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_flite.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qsemaphore.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QTextToSpeechEngineFlite::QTextToSpeechEngineFlite(const QVariantMap &parameters, QObject *parent)
    : QTextToSpeechEngine(parent)
{
    QAudioDevice audioDevice;
    if (const auto it = parameters.find(u"audioDevice"_s); it != parameters.end())
        audioDevice = (*it).value<QAudioDevice>();
    else
        audioDevice = QMediaDevices::defaultAudioOutput();

    if (audioDevice.isNull()) {
        m_errorReason = QTextToSpeech::ErrorReason::Playback;
        m_errorString = QCoreApplication::translate("QTextToSpeech", "No audio device available");
    }
    m_processor = std::make_unique<QTextToSpeechProcessorFlite>(audioDevice);

    // Connect processor to engine for state changes and error
    connect(m_processor.get(), &QTextToSpeechProcessorFlite::stateChanged,
            this, &QTextToSpeechEngineFlite::changeState);
    connect(m_processor.get(), &QTextToSpeechProcessorFlite::errorOccurred, this,
            &QTextToSpeechEngineFlite::setError);
    connect(m_processor.get(), &QTextToSpeechProcessorFlite::sayingWord, this,
            &QTextToSpeechEngine::sayingWord);
    connect(m_processor.get(), &QTextToSpeechProcessorFlite::synthesized, this,
            &QTextToSpeechEngine::synthesized);

    // Read voices from processor before moving it to a separate thread
    const QList<QTextToSpeechProcessorFlite::VoiceInfo> voices = m_processor->voices();

    int voiceIndex = 0;
    for (const QTextToSpeechProcessorFlite::VoiceInfo &voiceInfo : voices) {
        const QLocale locale(voiceInfo.locale);
        const QVoice voice = QTextToSpeechEngine::createVoice(voiceInfo.name, locale,
                                                              voiceInfo.gender, voiceInfo.age,
                                                              QVariant(voiceInfo.id));
        m_voices.insert(locale, voice);
        // Use the first available locale/voice as a fallback
        if (voiceIndex == 0)
            m_voice = voice;
        ++voiceIndex;
    }

    if (voiceIndex) {
        m_state = QTextToSpeech::Ready;
        m_thread.setObjectName("QTextToSpeechEngineFlite");
        m_processor->moveToThread(&m_thread);
        m_thread.start();
        m_thread.setPriority(QThread::HighestPriority); // we feed data to the audio sink
    } else {
        m_errorReason = QTextToSpeech::ErrorReason::Configuration;
        m_errorString = QCoreApplication::translate("QTextToSpeech", "No voices available");
    }
}

QTextToSpeechEngineFlite::~QTextToSpeechEngineFlite()
{
    if (m_processor->thread() != thread()) {
        QMetaObject::invokeMethod(m_processor.get(), [&] {
            m_processor.reset(); // ensure destruction on the correct thread
        }, Qt::BlockingQueuedConnection);
    }

    m_thread.exit();
    m_thread.wait();
}

QList<QLocale> QTextToSpeechEngineFlite::availableLocales() const
{
    return m_voices.uniqueKeys();
}

QList<QVoice> QTextToSpeechEngineFlite::availableVoices() const
{
    return m_voices.values(m_voice.locale());
}

void QTextToSpeechEngineFlite::say(const QString &text)
{
    QMetaObject::invokeMethod(m_processor.get(), "say", Qt::QueuedConnection, Q_ARG(QString, text),
                              Q_ARG(int, voiceData(voice()).toInt()), Q_ARG(double, pitch()),
                              Q_ARG(double, rate()), Q_ARG(double, volume()));
}

void QTextToSpeechEngineFlite::synthesize(const QString &text)
{
    QMetaObject::invokeMethod(m_processor.get(), "synthesize", Qt::QueuedConnection, Q_ARG(QString, text),
                              Q_ARG(int, voiceData(voice()).toInt()), Q_ARG(double, pitch()),
                              Q_ARG(double, rate()), Q_ARG(double, volume()));
}

void QTextToSpeechEngineFlite::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    QMetaObject::invokeMethod(m_processor.get(), &QTextToSpeechProcessorFlite::stop,
                              Qt::QueuedConnection, boundaryHint);
}

void QTextToSpeechEngineFlite::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    QMetaObject::invokeMethod(m_processor.get(), &QTextToSpeechProcessorFlite::pause,
                              Qt::QueuedConnection, boundaryHint);
}

void QTextToSpeechEngineFlite::resume()
{
    QMetaObject::invokeMethod(m_processor.get(), &QTextToSpeechProcessorFlite::resume, Qt::QueuedConnection);
}

double QTextToSpeechEngineFlite::rate() const
{
    return m_rate;
}

bool QTextToSpeechEngineFlite::setRate(double rate)
{
    if (m_rate == rate)
        return false;

    m_rate = rate;
    return true;
}

double QTextToSpeechEngineFlite::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineFlite::setPitch(double pitch)
{
    if (m_pitch == pitch)
        return false;

    m_pitch = pitch;
    return true;
}

QLocale QTextToSpeechEngineFlite::locale() const
{
    return m_voice.locale();
}

bool QTextToSpeechEngineFlite::setLocale(const QLocale &locale)
{
    const auto &voices = m_voices.values(locale);
    if (voices.isEmpty())
        return false;
    // The list returned by QMultiHash::values is reversed
    setVoice(voices.last());
    return true;
}

double QTextToSpeechEngineFlite::volume() const
{
    return m_volume;
}

bool QTextToSpeechEngineFlite::setVolume(double volume)
{
    if (m_volume == volume)
        return false;

    m_volume = volume;
    return true;
}

QVoice QTextToSpeechEngineFlite::voice() const
{
    return m_voice;
}

bool QTextToSpeechEngineFlite::setVoice(const QVoice &voice)
{
    QLocale locale = m_voices.key(voice); // returns default locale if not found, so
    if (!m_voices.contains(locale, voice)) {
        qWarning() << "Voice" << voice << "is not supported by this engine";
        return false;
    }

    m_voice = voice;
    return true;
}

void QTextToSpeechEngineFlite::changeState(QTextToSpeech::State newState)
{
    if (newState != m_state) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

QTextToSpeech::State QTextToSpeechEngineFlite::state() const
{
    return m_state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineFlite::errorReason() const
{
    return m_errorReason;
}

QString QTextToSpeechEngineFlite::errorString() const
{
    return m_errorString;
}

void QTextToSpeechEngineFlite::setError(QTextToSpeech::ErrorReason error, const QString &errorString)
{
    m_errorReason = error;
    m_errorString = errorString;
    changeState(QTextToSpeech::Error);
    emit errorOccurred(error, errorString);
}

QT_END_NAMESPACE
