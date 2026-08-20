// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_FLITE_H
#define QTEXTTOSPEECHENGINE_FLITE_H

#include "qtexttospeech_flite_processor.h"
#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QMultiHash>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineFlite : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineFlite(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineFlite() override;

    // Plug-in API:
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

Q_SIGNALS:
    void speaking();
    void engineErrorOccurred(QTextToSpeech::ErrorReason, const QString &errorString);

private slots:
    void changeState(QTextToSpeech::State newState);
    void setError(QTextToSpeech::ErrorReason error, const QString &errorString);

private:
    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;

    QVoice m_voice;
    double m_rate = 0;
    double m_pitch = 1;
    double m_volume = 1;

    // Voices mapped by their locale name.
    QMultiHash<QLocale, QVoice> m_voices;

    // Thread for blocking operations
    QThread m_thread;
    std::unique_ptr<QTextToSpeechProcessorFlite> m_processor;
};

QT_END_NAMESPACE

#endif
