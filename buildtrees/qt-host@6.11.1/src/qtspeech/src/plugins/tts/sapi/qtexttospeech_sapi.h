// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_SAPI_H
#define QTEXTTOSPEECHENGINE_SAPI_H

#include <QtCore/qt_windows.h>
#include <sapi.h>

#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qhash.h>
#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineSapi : public QTextToSpeechEngine, public ISpNotifyCallback
{
    Q_OBJECT

public:
    QTextToSpeechEngineSapi(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineSapi();

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

    HRESULT STDMETHODCALLTYPE NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/) override;
    friend class OputStream;

private:
    bool isSpeaking() const;
    QMap<QString, QString> voiceAttributes(ISpObjectToken *speechToken) const;
    QString voiceId(ISpObjectToken *speechToken) const;
    QLocale lcidToLocale(const QString &lcid) const;
    QVoice::Age toVoiceAge(const QString &age) const;
    void updateVoices();
    void setError(QTextToSpeech::ErrorReason reason, const QString &string);

    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;
    QVoice m_currentVoice;
    // Voices mapped by their locale name.
    QMultiHash<QLocale, QVoice> m_voices;

    QString currentText;
    qsizetype textOffset = 0;
    ISpVoice *m_voice = nullptr;
    double m_pitch = 0.0;
    bool m_pauseRequested = false;
};
QT_END_NAMESPACE

#endif
