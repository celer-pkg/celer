// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_DARWIN_H
#define QTEXTTOSPEECHENGINE_DARWIN_H

#include <QtCore/qlist.h>
#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

Q_FORWARD_DECLARE_OBJC_CLASS(AVSpeechSynthesizer);
Q_FORWARD_DECLARE_OBJC_CLASS(AVSpeechSynthesisVoice);
Q_FORWARD_DECLARE_OBJC_CLASS(AVSpeechUtterance);

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineDarwin : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineDarwin(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineDarwin();

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

    void setState(QTextToSpeech::State state);

    bool ignoreNextUtterance = false;
private:
    AVSpeechSynthesisVoice *fromQVoice(const QVoice &voice) const;
    QVoice toQVoice(AVSpeechSynthesisVoice *avVoice) const;
    void setError(QTextToSpeech::ErrorReason reason, const QString &string);
    AVSpeechUtterance *prepareUtterance(const QString &text);

    AVSpeechSynthesizer *m_speechSynthesizer;
    QVoice m_voice;
    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;
    QAudioFormat m_format;

    double m_pitch = 0.0;
    double m_actualPitch = 1.0;
    double m_rate = 0.0;
    double m_volume = 1.0;
};

QT_END_NAMESPACE

#endif
