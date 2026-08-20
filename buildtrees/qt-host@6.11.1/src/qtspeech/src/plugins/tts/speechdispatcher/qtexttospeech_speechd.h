// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_SPEECHD_H
#define QTEXTTOSPEECHENGINE_SPEECHD_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <libspeechd.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineSpeechd : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineSpeechd(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineSpeechd();

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

    void spdStateChanged(SPDNotificationType state);

private:
    QLocale localeForVoice(SPDVoice *voice) const;
    bool connectToSpeechDispatcher();
    void updateVoices();
    void setError(QTextToSpeech::ErrorReason reason, const QString &errorString);

    QTextToSpeech::State m_state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;
    SPDConnection *speechDispatcher;
    QVoice m_currentVoice;
    // Voices mapped by their locale name.
    QMultiHash<QLocale, QVoice> m_voices;
};

QT_END_NAMESPACE

#endif
