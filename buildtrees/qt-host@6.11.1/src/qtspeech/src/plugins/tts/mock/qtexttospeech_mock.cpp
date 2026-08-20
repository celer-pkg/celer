// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_mock.h"

#include <QtCore/QTimerEvent>
#include <QtCore/QTimer>
#include <QtCore/qregularexpression.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QTextToSpeechEngineMock::QTextToSpeechEngineMock(const QVariantMap &parameters, QObject *parent)
    : QTextToSpeechEngine(parent), m_parameters(parameters)
{
    m_locale = availableLocales().first();
    m_voice = availableVoices().first();
    if (m_parameters[u"delayedInitialization"_s].toBool()) {
        QTimer::singleShot(50, this, [this]{
            m_state = QTextToSpeech::Ready;
            emit stateChanged(m_state);
        });
    } else {
        m_state = QTextToSpeech::Ready;
    }
    m_errorReason = QTextToSpeech::ErrorReason::NoError;
}

QTextToSpeechEngineMock::~QTextToSpeechEngineMock()
{
}

QList<QLocale> QTextToSpeechEngineMock::availableLocales() const
{
    QList<QLocale> locales;

    if (const auto it = m_parameters.find("voices"); it != m_parameters.constEnd()) {
        using VoiceData = QList<std::tuple<QString, QLocale, QVoice::Gender, QVoice::Age>>;
        const auto voicesData = it->value<VoiceData>();
        QSet<QLocale> localeSet;
        for (const auto &voiceData : voicesData)
            localeSet.insert(std::get<1>(voiceData));
        locales = localeSet.values();
    } else {
        locales << QLocale(QLocale::English, QLocale::UnitedKingdom)
                << QLocale(QLocale::English, QLocale::UnitedStates)
                << QLocale(QLocale::NorwegianBokmal, QLocale::Norway)
                << QLocale(QLocale::NorwegianNynorsk, QLocale::Norway)
                << QLocale(QLocale::Finnish, QLocale::Finland);
    }

    return locales;
}

QList<QVoice> QTextToSpeechEngineMock::availableVoices() const
{
    QList<QVoice> voices;

    if (const auto it = m_parameters.find("voices"); it != m_parameters.constEnd()) {
        const auto voicesData = it->value<QList<std::tuple<QString, QLocale, QVoice::Gender, QVoice::Age>>>();
        for (const auto &voiceData : voicesData) {
            const QLocale &voiceLocale = std::get<1>(voiceData);
            if (voiceLocale == m_locale) {
                voices << createVoice(std::get<0>(voiceData),
                                      voiceLocale,
                                      std::get<2>(voiceData),
                                      std::get<3>(voiceData),
                                      u"%1-%2"_s.arg(m_locale.bcp47Name()).arg(voices.count() + 1));
            }
        }
    } else {
        const QString voiceData = m_locale.bcp47Name();
        const auto newVoice = [this, &voiceData](const QString &name, QVoice::Gender gender,
                                  QVoice::Age age, const char *suffix) {
            return createVoice(name, m_locale, gender, age,
                               QVariant::fromValue<QString>(voiceData + suffix));
        };
        switch (m_locale.language()) {
        case QLocale::English: {
            if (m_locale.territory() == QLocale::UnitedKingdom) {
                voices << newVoice("Bob", QVoice::Male, QVoice::Adult, "-1")
                       << newVoice("Anne", QVoice::Female, QVoice::Adult, "-2");
            } else {
                voices << newVoice("Charly", QVoice::Male, QVoice::Senior, "-1")
                       << newVoice("Mary", QVoice::Female, QVoice::Teenager, "-2");
            }
            break;
        }
        case QLocale::NorwegianBokmal:
            voices << newVoice("Eivind", QVoice::Male, QVoice::Adult, "-1")
                   << newVoice("Kjersti", QVoice::Female, QVoice::Adult, "-2");
            break;
        case QLocale::NorwegianNynorsk:
            voices << newVoice("Anders", QVoice::Male, QVoice::Teenager, "-1")
                   << newVoice("Ingvild", QVoice::Female, QVoice::Child, "-2");
            break;
        case QLocale::Finnish:
            voices << newVoice("Kari", QVoice::Male, QVoice::Adult, "-1")
                   << newVoice("Anneli", QVoice::Female, QVoice::Adult, "-2");
            break;
        default:
            Q_ASSERT_X(false, "availableVoices", "Unsupported locale!");
            break;
        }
    }
    return voices;
}

void QTextToSpeechEngineMock::say(const QString &text)
{
    m_text = text;
    m_currentIndex = 0;
    m_timer.start(wordTime(), Qt::PreciseTimer, this);
    m_state = QTextToSpeech::Speaking;
    emit stateChanged(m_state);
}

void QTextToSpeechEngineMock::synthesize(const QString &text)
{
    m_text = text;
    m_currentIndex = 0;
    m_timer.start(wordTime(), Qt::PreciseTimer, this);
    m_state = QTextToSpeech::Synthesizing;
    emit stateChanged(m_state);

    m_format.setSampleRate(22050);
    m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

void QTextToSpeechEngineMock::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (m_state == QTextToSpeech::Ready || m_state == QTextToSpeech::Error)
        return;

    Q_ASSERT(m_state == QTextToSpeech::Paused || m_timer.isActive());
    // finish immediately
    m_text.clear();
    m_currentIndex = -1;
    m_timer.stop();

    m_state = QTextToSpeech::Ready;
    emit stateChanged(m_state);
}

void QTextToSpeechEngineMock::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (m_state != QTextToSpeech::Speaking)
        return;

    // implement "pause after word end"
    m_pauseRequested = true;
}

void QTextToSpeechEngineMock::resume()
{
    if (m_state != QTextToSpeech::Paused)
        return;

    m_timer.start(wordTime(), Qt::PreciseTimer, this);
    m_state = QTextToSpeech::Speaking;
    emit stateChanged(m_state);
}

void QTextToSpeechEngineMock::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_timer.timerId()) {
        QTextToSpeechEngine::timerEvent(e);
        return;
    }

    Q_ASSERT(m_state == QTextToSpeech::Speaking || m_state == QTextToSpeech::Synthesizing);
    Q_ASSERT(m_text.length());

    // Find start of next word, skipping punctuations. This is good enough for testing.
    QRegularExpressionMatch match;
    qsizetype nextSpace = m_text.indexOf(QRegularExpression(u"\\W+"_s), m_currentIndex, &match);
    if (nextSpace == -1)
        nextSpace = m_text.length();
    const QString word = m_text.sliced(m_currentIndex, nextSpace - m_currentIndex);
    sayingWord(word, m_currentIndex, nextSpace - m_currentIndex);
    m_currentIndex = nextSpace + match.captured().length();

    emit synthesized(m_format, QByteArray(m_format.bytesForDuration(wordTime() * 1000), 0));

    if (m_currentIndex >= m_text.length()) {
        // done speaking all words
        m_timer.stop();
        m_state = QTextToSpeech::Ready;
        m_currentIndex = -1;
        emit stateChanged(m_state);
    } else if (m_pauseRequested) {
        m_timer.stop();
        m_state = QTextToSpeech::Paused;
        emit stateChanged(m_state);
    }
    m_pauseRequested = false;
}

double QTextToSpeechEngineMock::rate() const
{
    return m_rate;
}

bool QTextToSpeechEngineMock::setRate(double rate)
{
    m_rate = rate;
    if (m_timer.isActive()) {
        m_timer.stop();
        m_timer.start(wordTime(), Qt::PreciseTimer, this);
    }
    return true;
}

double QTextToSpeechEngineMock::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineMock::setPitch(double pitch)
{
    m_pitch = pitch;
    return true;
}

QLocale QTextToSpeechEngineMock::locale() const
{
    return m_locale;
}

bool QTextToSpeechEngineMock::setLocale(const QLocale &locale)
{
    if (!availableLocales().contains(locale))
        return false;
    m_locale = locale;
    const auto voices = availableVoices();
    if (!voices.contains(m_voice))
        m_voice = voices.isEmpty() ? QVoice() : voices.first();
    return true;
}

double QTextToSpeechEngineMock::volume() const
{
    return m_volume;
}

bool QTextToSpeechEngineMock::setVolume(double volume)
{
    if (volume < 0.0 || volume > 1.0)
        return false;

    m_volume = volume;
    return true;
}

QVoice QTextToSpeechEngineMock::voice() const
{
    return m_voice;
}

bool QTextToSpeechEngineMock::setVoice(const QVoice &voice)
{
    const QString voiceId = voiceData(voice).toString();
    const QLocale voiceLocale = QLocale(voiceId.left(voiceId.lastIndexOf("-")));
    if (!availableLocales().contains(voiceLocale)) {
        qWarning("Engine does not support voice's locale %s",
                 qPrintable(voiceLocale.bcp47Name()));
        return false;
    }
    m_locale = voiceLocale;
    if (!availableVoices().contains(voice)) {
        qWarning("Engine does not support voice %s in the locale %s",
                 qPrintable(voice.name()), qPrintable(voiceLocale.bcp47Name()));
        return false;
    }
    m_voice = voice;
    return true;
}

QTextToSpeech::State QTextToSpeechEngineMock::state() const
{
    return m_state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineMock::errorReason() const
{
    return m_errorReason;
}

QString QTextToSpeechEngineMock::errorString() const
{
    return m_errorString;
}

QT_END_NAMESPACE
