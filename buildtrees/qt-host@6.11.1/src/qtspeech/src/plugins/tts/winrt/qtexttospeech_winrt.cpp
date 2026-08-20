// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_winrt.h"
#include "qtexttospeech_winrt_audiosource.h"

#include <QtMultimedia/qaudiodevice.h>
#include <QtMultimedia/qaudiosink.h>
#include <QtMultimedia/qmediadevices.h>
#include <QtMultimedia/private/qaudiosystem_p.h>

#include <QtCore/QBasicTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/qset.h>
#include <QtCore/private/qfunctions_winrt_p.h>

#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.media.speechsynthesis.h>
#include <windows.storage.streams.h>

#include <wrl.h>

using namespace Qt::StringLiterals;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Media::SpeechSynthesis;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineWinRTPrivate
{
    Q_DECLARE_PUBLIC(QTextToSpeechEngineWinRT);
public:
    QTextToSpeechEngineWinRTPrivate(QTextToSpeechEngineWinRT *q);
    ~QTextToSpeechEngineWinRTPrivate();

    void setError(QTextToSpeech::ErrorReason reason, const QString &string);
    QTextToSpeech::State state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString errorString;

    // interfaces used to access the speech synthesizer
    ComPtr<ISpeechSynthesizer> synth;
    ComPtr<ISpeechSynthesizerOptions2> options;

    // data streaming - AudioSource implements COM interfaces as well as
    // QIODevice, so we store it in a ComPtr instead of a std::unique_ptr
    ComPtr<AudioSource> audioSource;
    QList<AudioSource::Boundary> boundaries;
    QList<AudioSource::Boundary>::const_iterator currentBoundary;
    QBasicTimer boundaryTimer;
    QElapsedTimer elapsedTimer;
    qint64 playedTime = 0;
    // the sink is connected to the source
    std::unique_ptr<QAudioSink> audioSink;

    template <typename Fn> void forEachVoice(Fn &&lambda) const;
    void updateVoices();
    QVoice createVoiceForInformation(const ComPtr<IVoiceInformation> &info) const;
    void initializeAudioSink(const QAudioFormat &format);
    void sinkStateChanged(QAudio::State sinkState);

private:
    QAudioDevice audioDevice;
    QTextToSpeechEngineWinRT *q_ptr;
};


QTextToSpeechEngineWinRTPrivate::QTextToSpeechEngineWinRTPrivate(QTextToSpeechEngineWinRT *q)
    : q_ptr(q)
{
}

QTextToSpeechEngineWinRTPrivate::~QTextToSpeechEngineWinRTPrivate()
{
    // Close and free the source explicitly and in the right order so that the buffer's
    // aboutToClose signal gets emitted before this private object is destroyed.
    if (audioSource) {
        audioSource->close();
        audioSource.Reset();
    }
}

void QTextToSpeechEngineWinRTPrivate::setError(QTextToSpeech::ErrorReason reason, const QString &string)
{
    Q_Q(QTextToSpeechEngineWinRT);
    errorReason = reason;
    errorString = string;
    if (reason != QTextToSpeech::ErrorReason::NoError)
        return;
    if (state != QTextToSpeech::Error) {
        state = QTextToSpeech::Error;
        emit q->stateChanged(state);
    }
    emit q->errorOccurred(errorReason, errorString);
}

QTextToSpeechEngineWinRT::QTextToSpeechEngineWinRT(const QVariantMap &params, QObject *parent)
    : QTextToSpeechEngine(parent)
    , d_ptr(new QTextToSpeechEngineWinRTPrivate(this))
{
    Q_D(QTextToSpeechEngineWinRT);

    if (const auto it = params.find("audioDevice"_L1); it != params.end())
        d->audioDevice = (*it).value<QAudioDevice>();
    else
        d->audioDevice = QMediaDevices::defaultAudioOutput();

    if (d->audioDevice.isNull())
        d->setError(QTextToSpeech::ErrorReason::Playback,
                    QCoreApplication::translate("QTextToSpeech", "No audio device available."));

    HRESULT hr = CoInitialize(nullptr);
    Q_ASSERT_SUCCEEDED(hr);

    hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Media_SpeechSynthesis_SpeechSynthesizer).Get(),
                            &d->synth);
    if (!SUCCEEDED(hr)) {
        d->setError(QTextToSpeech::ErrorReason::Initialization,
                    QCoreApplication::translate("QTextToSpeech", "Could not initialize text-to-speech engine."));
        return;
    } else if (voice() == QVoice()) {
        d->setError(QTextToSpeech::ErrorReason::Configuration,
                    QCoreApplication::translate("QTextToSpeech", "Could not set default voice."));
    } else {
        d->state = QTextToSpeech::Ready;
        d->errorReason = QTextToSpeech::ErrorReason::NoError;
    }

    // the rest is optional, we might not support these features

    ComPtr<ISpeechSynthesizer2> synth2;
    hr = d->synth->QueryInterface(__uuidof(ISpeechSynthesizer2), &synth2);
    RETURN_VOID_IF_FAILED("ISpeechSynthesizer2 not implemented.");

    ComPtr<ISpeechSynthesizerOptions> options1;
    hr = synth2->get_Options(&options1);
    Q_ASSERT_SUCCEEDED(hr);

    // ask for boundary data, we might not get it
    options1->put_IncludeSentenceBoundaryMetadata(true);
    options1->put_IncludeWordBoundaryMetadata(true);

    hr = options1->QueryInterface(__uuidof(ISpeechSynthesizerOptions2), &d->options);
    RETURN_VOID_IF_FAILED("ISpeechSynthesizerOptions2 not implemented.");

    d->options->put_AudioPitch(1.0);
    d->options->put_AudioVolume(1.0);
    d->options->put_SpeakingRate(1.0);
}

QTextToSpeechEngineWinRT::~QTextToSpeechEngineWinRT()
{
    d_ptr.reset();
    CoUninitialize();
}

/* Voice and language/locale management */

QVoice QTextToSpeechEngineWinRTPrivate::createVoiceForInformation(const ComPtr<IVoiceInformation> &info) const
{
    Q_Q(const QTextToSpeechEngineWinRT);
    HRESULT hr;

    HString nativeName;
    hr = info->get_DisplayName(nativeName.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    VoiceGender gender;
    hr = info->get_Gender(&gender);
    Q_ASSERT_SUCCEEDED(hr);

    HString voiceLanguage;
    hr = info->get_Language(voiceLanguage.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    HString voiceId;
    hr = info->get_Id(voiceId.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    return q->createVoice(QString::fromWCharArray(nativeName.GetRawBuffer(0)),
                          QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0))),
                          gender == VoiceGender_Male ? QVoice::Male : QVoice::Female,
                          QVoice::Other,
                          QString::fromWCharArray(voiceId.GetRawBuffer(0)));
}

/*
    Iterates over all available voices and calls \a lambda with the
    IVoiceInformation for each. If the lambda returns true, the iteration
    ends.

    This helper is used in all voice and locale related engine implementations.
    While not particular fast, none of these functions are performance critical,
    and always operating on the official list of all voices avoids that we need
    to maintain our own mappings, or keep all voice information instances in
    memory.
*/
template <typename Fn>
void QTextToSpeechEngineWinRTPrivate::forEachVoice(Fn &&lambda) const
{
    HRESULT hr;

    ComPtr<IInstalledVoicesStatic> stat;
    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Media_SpeechSynthesis_SpeechSynthesizer).Get(),
                                IID_PPV_ARGS(&stat));
    Q_ASSERT_SUCCEEDED(hr);

    ComPtr<IVectorView<VoiceInformation*>> voiceInformations;
    hr = stat->get_AllVoices(&voiceInformations);
    RETURN_VOID_IF_FAILED("Could not get voice information.");

    quint32 voiceSize;
    hr = voiceInformations->get_Size(&voiceSize);
    RETURN_VOID_IF_FAILED("Could not access size of voice information.");

    for (quint32 i = 0; i < voiceSize; ++i) {
        ComPtr<IVoiceInformation> voiceInfo;
        hr = voiceInformations->GetAt(i, &voiceInfo);
        Q_ASSERT_SUCCEEDED(hr);

        if (lambda(voiceInfo))
            break;
    }
}

QList<QLocale> QTextToSpeechEngineWinRT::availableLocales() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    if (!d->synth)
        return QList<QLocale>();
    QSet<QLocale> uniqueLocales;
    d->forEachVoice([&uniqueLocales](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceLanguage;
        HRESULT hr = voiceInfo->get_Language(voiceLanguage.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);

        uniqueLocales.insert(QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0))));
        return false;
    });
    return uniqueLocales.values();
}

QList<QVoice> QTextToSpeechEngineWinRT::availableVoices() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    if (!d->synth)
        return QList<QVoice>();
    QList<QVoice> voices;
    const QLocale currentLocale = locale();
    d->forEachVoice([&](const ComPtr<IVoiceInformation> &voiceInfo) {
        const QVoice voice = d->createVoiceForInformation(voiceInfo);
        if (currentLocale == voice.locale())
            voices.append(voice);
        return false;
    });
    return voices;
}

QLocale QTextToSpeechEngineWinRT::locale() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    if (!d->synth)
        return QLocale(QLocale::C, QLocale::AnyTerritory);

    ComPtr<IVoiceInformation> voiceInfo;
    HRESULT hr = d->synth->get_Voice(&voiceInfo);
    if (FAILED(hr) || !voiceInfo)
        return QLocale(QLocale::C, QLocale::AnyTerritory);

    HString language;
    hr = voiceInfo->get_Language(language.GetAddressOf());
    if (FAILED(hr))
        return QLocale(QLocale::C, QLocale::AnyTerritory);

    return QLocale(QString::fromWCharArray(language.GetRawBuffer(0)));
}

bool QTextToSpeechEngineWinRT::setLocale(const QLocale &locale)
{
    Q_D(QTextToSpeechEngineWinRT);
    if (!d->synth)
        return false;

    ComPtr<IVoiceInformation> foundVoice;
    d->forEachVoice([&locale, &foundVoice](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceLanguage;
        HRESULT hr = voiceInfo->get_Language(voiceLanguage.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);

        if (locale == QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0)))) {
            foundVoice = voiceInfo;
            return true;
        }
        return false;
    });

    if (!foundVoice) {
        d->setError(QTextToSpeech::ErrorReason::Configuration,
                    QCoreApplication::translate("QTextToSpeech", "No voice available for locale %1.")
                        .arg(locale.bcp47Name()));
        return false;
    }

    return SUCCEEDED(d->synth->put_Voice(foundVoice.Get()));
}

QVoice QTextToSpeechEngineWinRT::voice() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    if (!d->synth)
        return QVoice();

    ComPtr<IVoiceInformation> voiceInfo;
    d->synth->get_Voice(&voiceInfo);

    return d->createVoiceForInformation(voiceInfo);
}

bool QTextToSpeechEngineWinRT::setVoice(const QVoice &voice)
{
    Q_D(QTextToSpeechEngineWinRT);
    if (!d->synth)
        return false;

    const QString data = QTextToSpeechEngine::voiceData(voice).toString();
    if (data.isEmpty())
        d->setError(QTextToSpeech::ErrorReason::Configuration,
                    QCoreApplication::translate("QTextToSpeech", "Invalid voice."));

    ComPtr<IVoiceInformation> foundVoice;
    d->forEachVoice([&data, &foundVoice](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceId;
        HRESULT hr = voiceInfo->get_Id(voiceId.GetAddressOf());
        if (FAILED(hr))
            return false;

        if (data == QString::fromWCharArray(voiceId.GetRawBuffer(0))) {
            foundVoice = voiceInfo;
            return true;
        }
        return false;
    });

    if (!foundVoice) {
        d->setError(QTextToSpeech::ErrorReason::Configuration,
                    QCoreApplication::translate("QTextToSpeech", "Invalid voice."));
        return false;
    }

    return SUCCEEDED(d->synth->put_Voice(foundVoice.Get()));
}

void QTextToSpeechEngineWinRT::timerEvent(QTimerEvent *e)
{
    Q_D(QTextToSpeechEngineWinRT);
    if (e->timerId() == d->boundaryTimer.timerId()) {
        const qint64 elapsed = d->elapsedTimer.nsecsElapsed() / 1000 + d->playedTime;
        if (d->currentBoundary->type == AudioSource::Boundary::Word)
            emit sayingWord(d->currentBoundary->text, d->currentBoundary->beginIndex,
                            d->currentBoundary->endIndex - d->currentBoundary->beginIndex + 1);
        ++d->currentBoundary;

        if (d->audioSource && d->currentBoundary != d->boundaries.constEnd()) {
            const qint64 msecsToNext = qMax((d->currentBoundary->startTime - elapsed) / 1000, 0);
            d->boundaryTimer.start(msecsToNext, Qt::PreciseTimer, this);
        } else {
            d->boundaryTimer.stop();
        }
    }
    QObject::timerEvent(e);
}

/* State and speech control */

QTextToSpeech::State QTextToSpeechEngineWinRT::state() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineWinRT::errorReason() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->errorReason;
}

QString QTextToSpeechEngineWinRT::errorString() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->errorString;
}

void QTextToSpeechEngineWinRTPrivate::initializeAudioSink(const QAudioFormat &format)
{
    Q_Q(const QTextToSpeechEngineWinRT);

    // cancelled or another call to say() while waiting for the synthesizer
    if (!audioSource)
        return;

    boundaries = audioSource->boundaryData();
    currentBoundary = boundaries.constBegin();

    audioSink.reset(new QAudioSink(audioDevice, format));
    // LATER: use public API (compare QTBUG-138378)
    QPlatformAudioSink *platformAudioSink = QPlatformAudioSink::get(*audioSink);
    if (platformAudioSink)
        platformAudioSink->setRole(QPlatformAudioSink::AudioEndpointRole::Accessibility);

    QObject::connect(audioSink.get(), &QAudioSink::stateChanged,
                     q, [this](QAudio::State sinkState) {
        sinkStateChanged(sinkState);
    });
    audioSink->start(audioSource.Get());
}

void QTextToSpeechEngineWinRTPrivate::sinkStateChanged(QAudio::State sinkState)
{
    Q_Q(QTextToSpeechEngineWinRT);

    const auto oldState = state;
    switch (sinkState) {
    case QAudio::IdleState:
        if (audioSource) {
            if (audioSource->atEnd()) {
                state = QTextToSpeech::Ready;
                playedTime = 0;
                elapsedTimer.invalidate();
            } else {
                boundaryTimer.stop();
                if (elapsedTimer.isValid())
                    playedTime += elapsedTimer.nsecsElapsed() / 1000;
                elapsedTimer.invalidate();
                audioSink->suspend();
            }
        }
        break;
    case QAudio::StoppedState:
        state = QTextToSpeech::Ready;
        playedTime = 0;
        elapsedTimer.invalidate();
        break;
    case QAudio::ActiveState:
        // boundaries are in micro-seconds, timers have msec precision
        if (currentBoundary != boundaries.constEnd())
            boundaryTimer.start(qMax(currentBoundary->startTime - playedTime, 0) / 1000, Qt::PreciseTimer, q);
        elapsedTimer.start();
        state = QTextToSpeech::Speaking;
        break;
    case QAudio::SuspendedState:
        if (audioSource->m_pause != AudioSource::NoPause)
            state = QTextToSpeech::Paused;
        break;
    }
    if (state != oldState)
        emit q->stateChanged(state);
}

void QTextToSpeechEngineWinRT::say(const QString &text)
{
    Q_D(QTextToSpeechEngineWinRT);
    if (!d->synth)
        return;

    // stop ongoing speech
    stop(QTextToSpeech::BoundaryHint::Default);

    HRESULT hr = S_OK;

    HStringReference nativeText(reinterpret_cast<LPCWSTR>(text.utf16()), text.length());

    ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation;
    hr = d->synth->SynthesizeTextToStreamAsync(nativeText.Get(), &synthOperation);
    if (!SUCCEEDED(hr)) {
        d->setError(QTextToSpeech::ErrorReason::Input,
                    QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
        return;
    }

    // The source will wait for the the data resulting out of the synthOperation, and emits
    // streamReady when data is available. This starts a QAudioSink, which pulls the data.
    d->audioSource.Attach(new AudioSource(synthOperation));

    connect(d->audioSource.Get(), &AudioSource::streamReady, this, [d](const QAudioFormat &format){
        d->initializeAudioSink(format);
    });
    connect(d->audioSource.Get(), &AudioSource::errorInStream, this, [this]{
        Q_D(QTextToSpeechEngineWinRT);
        d->setError(QTextToSpeech::ErrorReason::Playback,
                    QCoreApplication::translate("QTextToSpeech", "Error in audio stream."));
    });
    connect(d->audioSource.Get(), &QIODevice::aboutToClose, this, [d]{
        d->boundaryTimer.stop();
        d->audioSink.reset();
    });
}

void QTextToSpeechEngineWinRT::synthesize(const QString &text)
{
    Q_D(QTextToSpeechEngineWinRT);

    HRESULT hr = S_OK;

    HStringReference nativeText(reinterpret_cast<LPCWSTR>(text.utf16()), text.length());

    ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation;
    hr = d->synth->SynthesizeTextToStreamAsync(nativeText.Get(), &synthOperation);
    if (!SUCCEEDED(hr)) {
        d->setError(QTextToSpeech::ErrorReason::Input,
                    QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
        return;
    }

    // The source will wait for the the data resulting out of the synthOperation, and emits
    // streamReady when data is available. This starts a QAudioSink, which pulls the data.
    d->audioSource.Attach(new AudioSource(synthOperation));

    connect(d->audioSource.Get(), &AudioSource::streamReady, this, [d, this](const QAudioFormat &format){
        if (d->state != QTextToSpeech::Synthesizing) {
            d->state = QTextToSpeech::Synthesizing;
            emit stateChanged(d->state);
        }
    });
    connect(d->audioSource.Get(), &AudioSource::readyRead, this, [d, this](){
        Q_ASSERT(d->state == QTextToSpeech::Synthesizing);
        const QByteArray data = d->audioSource->read(d->audioSource->bytesAvailable());
        emit synthesized(d->audioSource->format(), data);
        if (d->audioSource->atEnd())
            d->audioSource->close();
    });
    connect(d->audioSource.Get(), &AudioSource::aboutToClose, this, [d, this]{
        if (d->state != QTextToSpeech::Ready) {
            d->state = QTextToSpeech::Ready;
            emit stateChanged(d->state);
        }
    });
    connect(d->audioSource.Get(), &AudioSource::errorInStream, this, [d]{
        d->setError(QTextToSpeech::ErrorReason::Input,
                    QCoreApplication::translate("QTextToSpeech", "Error synthesizing audio stream."));
    });
}

void QTextToSpeechEngineWinRT::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    Q_D(QTextToSpeechEngineWinRT);

    if (d->audioSource) {
        d->audioSource->close();
        d->audioSource.Reset();
    }
}

void QTextToSpeechEngineWinRT::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->audioSource)
        return;

    auto pauseBoundaryType = AudioSource::Boundary::Unknown;
    switch (boundaryHint) {
    case QTextToSpeech::BoundaryHint::Default:
        d->audioSource->pause(0);
        return;
    case QTextToSpeech::BoundaryHint::Immediate:
        d->audioSource->pause(0);
        if (d->audioSink)
            d->audioSink->suspend();
        return;
    case QTextToSpeech::BoundaryHint::Word:
        pauseBoundaryType = AudioSource::Boundary::Word;
        break;
    case QTextToSpeech::BoundaryHint::Sentence:
        pauseBoundaryType = AudioSource::Boundary::Sentence;
        break;
    case QTextToSpeech::BoundaryHint::Utterance:
        // taken care off by engine-independent implementation
        return;
    }

    // find the next boundary of the matching type
    const auto nextBoundary = std::find_if(d->currentBoundary + 1, d->boundaries.constEnd(),
                                            [pauseBoundaryType](auto &&it){
        return it.type == pauseBoundaryType;
    });
    if (nextBoundary != d->boundaries.constEnd()) {
        d->audioSource->pause(d->audioSource->format()
                                        .bytesForDuration(nextBoundary->startTime));
    }
}

void QTextToSpeechEngineWinRT::resume()
{
    Q_D(QTextToSpeechEngineWinRT);

    if (d->audioSource)
        d->audioSource->resume();
    if (d->audioSink)
        d->audioSink->resume();
}

/* Properties */

/*
    The native value can range from 0.5 (half the default rate) to 6.0 (6x the default rate), inclusive.
    The default value is 1.0 (the "normal" speaking rate for the current voice).

    QTextToSpeech rate is from -1.0 to 1.0.
*/
double QTextToSpeechEngineWinRT::rate() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_SpeakingRate(&value);
    if (value < 1.0) // 0.5 to 1.0 maps to -1.0 to 0.0
        value = (value - 1.0) * 2.0;
    else // 1.0 to 6.0 maps to 0.0 to 1.0
        value = (value - 1.0) / 5.0;
    return value;
}

bool QTextToSpeechEngineWinRT::setRate(double rate)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    if (rate < 0.0) // -1.0 to 0.0 maps to 0.5 to 1.0
        rate = 0.5 * rate + 1.0;
    else // 0.0 to 1.0 maps to 1.0 to 6.0
        rate = rate * 5.0 + 1.0;
    HRESULT hr = d->options->put_SpeakingRate(rate);
    return SUCCEEDED(hr);
}

/*
    The native value can range from 0.0 (lowest pitch) to 2.0 (highest pitch), inclusive.
    The default value is 1.0.

    The QTextToSpeech value is from -1.0 to 1.0.
*/
double QTextToSpeechEngineWinRT::pitch() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_AudioPitch(&value);
    return value - 1.0;
}

bool QTextToSpeechEngineWinRT::setPitch(double pitch)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    HRESULT hr = d->options->put_AudioPitch(pitch + 1.0);
    return SUCCEEDED(hr);
}

/*
    The native value can range from 0.0 (lowest volume) to 1.0 (highest volume), inclusive.
    The default value is 1.0.

    This maps directly to the QTextToSpeech volume range.
*/
double QTextToSpeechEngineWinRT::volume() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_AudioVolume(&value);
    return value;
}

bool QTextToSpeechEngineWinRT::setVolume(double volume)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    HRESULT hr = d->options->put_AudioVolume(volume);
    return SUCCEEDED(hr);
}

QT_END_NAMESPACE
