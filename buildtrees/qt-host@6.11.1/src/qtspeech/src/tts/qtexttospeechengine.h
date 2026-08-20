// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHENGINE_H
#define QTEXTTOSPEECHENGINE_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the Qt TextToSpeech plugin API, with limited compatibility
// guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtTextToSpeech/qtexttospeech.h>

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QDir>
#include <QtMultimedia/QAudioFormat>

QT_BEGIN_NAMESPACE

class QAudioFormat;

class Q_TEXTTOSPEECH_EXPORT QTextToSpeechEngine : public QObject
{
    Q_OBJECT

public:
    explicit QTextToSpeechEngine(QObject *parent = nullptr);
    ~QTextToSpeechEngine();

    virtual QTextToSpeech::Capabilities capabilities() const
    {
        return QTextToSpeech::Capability::None;
    }
    virtual QList<QLocale> availableLocales() const = 0;
    virtual QList<QVoice> availableVoices() const = 0;

    virtual void say(const QString &text) = 0;
    virtual void synthesize(const QString &text) = 0;
    virtual void stop(QTextToSpeech::BoundaryHint boundaryHint) = 0;
    virtual void pause(QTextToSpeech::BoundaryHint boundaryHint) = 0;
    virtual void resume() = 0;

    virtual double rate() const = 0;
    virtual bool setRate(double rate) = 0;
    virtual double pitch() const = 0;
    virtual bool setPitch(double pitch) = 0;
    virtual QLocale locale() const = 0;
    virtual bool setLocale(const QLocale &locale) = 0;
    virtual double volume() const = 0;
    virtual bool setVolume(double volume) = 0;
    virtual QVoice voice() const = 0;
    virtual bool setVoice(const QVoice &voice) = 0;
    virtual QTextToSpeech::State state() const = 0;
    virtual QTextToSpeech::ErrorReason errorReason() const = 0;
    virtual QString errorString() const = 0;

protected:
    static QVoice createVoice(const QString &name, const QLocale &locale, QVoice::Gender gender,
                              QVoice::Age age, const QVariant &data);
    static QVariant voiceData(const QVoice &voice);

Q_SIGNALS:
    void stateChanged(QTextToSpeech::State state);
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);

    void sayingWord(const QString &word, qsizetype start, qsizetype length);
    void synthesized(const QAudioFormat &format, const QByteArray &data);
};

QT_END_NAMESPACE

#endif
