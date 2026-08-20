// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_WINRT_H
#define QTEXTTOSPEECHENGINE_WINRT_H

#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QAudioDevice;
class QTextToSpeechEngineWinRTPrivate;

class QTextToSpeechEngineWinRT : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineWinRT(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineWinRT();

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
    QScopedPointer<QTextToSpeechEngineWinRTPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QTextToSpeechEngineWinRT)
};

QT_END_NAMESPACE

#endif
