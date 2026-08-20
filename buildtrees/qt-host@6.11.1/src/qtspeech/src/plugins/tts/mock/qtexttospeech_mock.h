// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECH_MOCK_H
#define QTEXTTOSPEECH_MOCK_H

#include "qtexttospeechengine.h"
#include <QtCore/QBasicTimer>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineMock : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    explicit QTextToSpeechEngineMock(const QVariantMap &parameters, QObject *parent = nullptr);
    ~QTextToSpeechEngineMock();

    QList<QLocale> availableLocales() const override;
    QList<QVoice> availableVoices() const override;

    void say(const QString &text) override;
    void synthesize(const QString &text) override;
    void stop(QTextToSpeech::BoundaryHint boundaryHint) override;
    void pause(QTextToSpeech::BoundaryHint boundaryHint) override;
    void resume() override;

    double rate() const override;
    bool setRate(double rate) override;
    double pitch() const override;
    bool setPitch(double pitch) override;
    QLocale locale() const override;
    bool setLocale(const QLocale &locale) override;
    double volume() const override;
    bool setVolume(double volume) override;
    QVoice voice() const override;
    bool setVoice(const QVoice &voice) override;
    QTextToSpeech::State state() const override;
    QTextToSpeech::ErrorReason errorReason() const override;
    QString errorString() const override;

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    // mock engine uses 100ms per word, +/- 50ms depending on rate
    int wordTime() const { return 100 - int(50.0 * m_rate); }

    const QVariantMap m_parameters;
    QString m_text;
    QLocale m_locale;
    QVoice m_voice;
    QBasicTimer m_timer;
    double m_rate = 0.0;
    double m_pitch = 0.0;
    double m_volume = 0.5;
    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;
    bool m_pauseRequested = false;
    qsizetype m_currentIndex = -1;
    QAudioFormat m_format;
};

QT_END_NAMESPACE

#endif
