// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_ANDROID_H
#define QTEXTTOSPEECHENGINE_ANDROID_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QString>
#include <QtCore/qjniobject.h>
#include <QtCore/qjnitypes.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineAndroid : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineAndroid(const QVariantMap &parameters, QObject *parent);
    virtual ~QTextToSpeechEngineAndroid();

    // Plug-in API:
    QTextToSpeech::Capabilities capabilities() const override;
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

public Q_SLOTS:
    void processNotifyReady();
    void processNotifyError(int reason);
    void processNotifySpeaking();
    void processNotifyRangeStart(int start, int end, int frame);
    void processNotifyBeginSynthesis(const QAudioFormat &format);
    void processNotifyAudioAvailable(const QByteArray &bytes);

private:
    void setState(QTextToSpeech::State state);
    void setError(QTextToSpeech::ErrorReason reason, const QString &string);
    QVoice javaVoiceObjectToQVoice(QJniObject &obj) const;

    QJniObject m_speech;
    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;
    QString m_text;
    QAudioFormat m_format;
};

Q_DECLARE_JNI_CLASS(QtTextToSpeech, "org/qtproject/qt/android/speech/QtTextToSpeech")

QT_END_NAMESPACE

#endif
