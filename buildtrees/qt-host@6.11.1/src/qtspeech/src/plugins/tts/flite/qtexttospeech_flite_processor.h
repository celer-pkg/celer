// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHPROCESSOR_FLITE_H
#define QTEXTTOSPEECHPROCESSOR_FLITE_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/qabstracteventdispatcher.h>
#include <QtCore/qbasictimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qprocess.h>
#include <QtCore/qstring.h>
#include <QtCore/qthread.h>
#include <QtMultimedia/qaudiosink.h>
#include <QtMultimedia/qmediadevices.h>

#include <flite/flite.h>

#include <optional>

QT_BEGIN_NAMESPACE

class QFliteSynthesisProcess;

class QTextToSpeechProcessorFlite : public QObject
{
    Q_OBJECT

public:
    QTextToSpeechProcessorFlite(QAudioDevice audioDevice);
    ~QTextToSpeechProcessorFlite();

    struct VoiceInfo
    {
        int id;
        cst_voice *vox;
        void (*unregister_func)(cst_voice *vox);
        QString name;
        QString locale;
        QVoice::Gender gender;
        QVoice::Age age;
    };

    Q_INVOKABLE void say(const QString &text, int voiceId, double pitch, double rate, double volume);
    Q_INVOKABLE void synthesize(const QString &text, int voiceId, double pitch, double rate, double volume);
    Q_INVOKABLE void pause(QTextToSpeech::BoundaryHint boundaryHint);
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop(QTextToSpeech::BoundaryHint boundaryHint);

    const QList<QTextToSpeechProcessorFlite::VoiceInfo> &voices() const;

private:
    bool init();
    bool checkVoice(int voiceId);

    void setError(QTextToSpeech::ErrorReason err, const QString &errorString = QString());
    void updateState(QTextToSpeech::State);

Q_SIGNALS:
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);
    void stateChanged(QTextToSpeech::State);
    void sayingWord(const QString &word, qsizetype begin, qsizetype length);
    void synthesized(const QAudioFormat &format, const QByteArray &array);

private:
    QTextToSpeech::State m_state = {};

    std::unique_ptr<QAudioSink> m_audioSink;
    const QAudioDevice m_audioDevice;
    float m_volume = 1.f;
    std::optional<QAudioFormat> m_synthesisFormat;

    QList<VoiceInfo> m_voices;

    // synthesis process
    friend class QFliteSynthesisProcess;
    std::unique_ptr<QFliteSynthesisProcess> m_synthesisProcess;

    void prepareAudioSink(QAudioFormat);
};

QT_END_NAMESPACE

#endif
