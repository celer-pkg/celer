// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:data-parser, execute-external-code

#include "qtexttospeech_flite_processor.h"
#include "qtexttospeech_flite_plugin.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qlocale.h>
#include <QtCore/qmap.h>
#include <QtCore/qpointer.h>
#include <QtCore/qprocessordetection.h>
#include <QtCore/qspan.h>
#include <QtCore/qstring.h>
#include <QtCore/qthreadpool.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtMultimedia/private/qaudiohelpers_p.h>
#include <QtMultimedia/private/qaudiosystem_p.h>

#include <flite/flite.h>

#include <deque>
#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

namespace {

void setRateForVoice(cst_voice *voice, float rate)
{
    float stretch = 1.0;
    Q_ASSERT(rate >= -1.0 && rate <= 1.0);
    // Stretch multipliers taken from Speech Dispatcher
    if (rate < 0)
        stretch -= rate * 2;
    if (rate > 0)
        stretch -= rate * (100.0f / 175.0f);
    feat_set_float(voice->features, "duration_stretch", stretch);
}

void setPitchForVoice(cst_voice *voice, float pitch)
{
    float f0;
    Q_ASSERT(pitch >= -1.0 && pitch <= 1.0);
    // Conversion taken from Speech Dispatcher
    f0 = (pitch * 80) + 100;
    feat_set_float(voice->features, "int_f0_target_mean", f0);
}

// Read available flite voices
QStringList fliteAvailableVoices(const QString &libPrefix, const QString &langCode)
{
    // Read statically linked voices
    QStringList voices;
    for (const cst_val *v = flite_voice_list; v; v = val_cdr(v)) {
        cst_voice *voice = val_voice(val_car(v));
        voices.append(voice->name);
    }

    // Read available libraries
    static const QStringList ldPaths = [] {
        const QProcessEnvironment pe;
        QStringList ldPaths = pe.value(u"LD_LIBRARY_PATH"_s).split(u":"_s, Qt::SkipEmptyParts);
        if (ldPaths.isEmpty()) {
            ldPaths = QStringList{
                // Fedora-style lib64 library paths
                u"/usr/lib64"_s,
                u"/usr/local/lib64"_s,
                u"/lib64"_s,

                // Debian-style multi-arch library paths
#if defined(Q_PROCESSOR_ARM_V8)
#  if defined(__MUSL__)
                u"/usr/lib/aarch64-linux-musl"_s,
#  else
                u"/usr/lib/aarch64-linux-gnu"_s,
#  endif
#elif defined(Q_PROCESSOR_ARM_V7)
#  if defined(__MUSL__)
                u"/usr/lib/arm-linux-musleabihf"_s,
#  else
#    if defined(__ARM_PCS_VFP)
                u"/usr/lib/arm-linux-gnueabihf"_s,
#    else
                u"/usr/lib/arm-linux-gnueabi"_s,
#    endif
#  endif
#elif defined(Q_PROCESSOR_X86_64)
                u"/usr/lib/x86_64-linux-gnu"_s,
#elif defined(Q_PROCESSOR_X86)
                u"/usr/lib/i686-linux-gnu"_s,
                u"/usr/lib/i386-linux-gnu"_s,
#endif

                // generic paths
                u"/usr/lib"_s,
                u"/usr/local/lib"_s,
                u"/lib"_s,
            };
        } else {
            ldPaths.removeDuplicates();
        }

        ldPaths.removeIf([](const QString &path) {
            QDir dir(path);
            return !dir.isReadable() || dir.isEmpty();
        });

        qCDebug(lcSpeechTtsFlite) << "QTextToSpeechProcessorFlite: initialized voice paths to"
                                  << ldPaths;

        return ldPaths;
    }();

    const QString libPattern = QString(u"lib"_s + libPrefix).arg(langCode).arg("*"_L1);
    for (const auto &path : ldPaths) {
        QDir dir(path);
        dir.setNameFilters({ libPattern });
        dir.setFilter(QDir::Files);
        const QFileInfoList fileList = dir.entryInfoList();
        for (const auto &file : fileList) {
            QString vox = file.fileName().mid(16, file.fileName().indexOf(u'.') - 16);
            voices.append(std::move(vox));
        }
    }

    voices.removeDuplicates();
    return voices;
}

QAudioFormat getAudioFormat(const cst_wave &w)
{
    QAudioFormat fmt;
    fmt.setSampleFormat(QAudioFormat::Int16);
    fmt.setSampleRate(w.sample_rate);
    fmt.setChannelCount(w.num_channels);
    fmt.setChannelConfig(QAudioFormat::defaultChannelConfigForChannelCount(w.num_channels));
    return fmt;
}

// we use a dedicated thread pool for flite synthesis:
// * it has a higher priority than the system thread pool
// * synthesizing multiple voices in parallel does not really make sense, so we limit it to 2
//   threads (it will typically only be one)
std::shared_ptr<QThreadPool> getFliteThreadPool()
{
    static std::weak_ptr<QThreadPool> singleton;
    static QMutex mutex;
    std::lock_guard guard{ mutex };
    std::shared_ptr<QThreadPool> pool = singleton.lock();
    if (pool)
        return pool;

    pool = std::make_shared<QThreadPool>();
    pool->setMaxThreadCount(2);
    pool->setThreadPriority(QThread::HighPriority);
    pool->setObjectName(u"QFliteThreadPool"_s);

    singleton = pool;
    return pool;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

class QFliteSynthesisProcess final : public QIODevice
{
    struct TokenInformation
    {
        QString word;
        std::chrono::milliseconds startTime;
    };

    using BoundaryHint = QTextToSpeech::BoundaryHint;

public:
    QFliteSynthesisProcess(cst_voice *voice, QTextToSpeechProcessorFlite *parent, QString text,
                           float pitch, float rate);
    ~QFliteSynthesisProcess();

    void pause(QTextToSpeech::BoundaryHint boundaryHint);
    void stop(QTextToSpeech::BoundaryHint boundaryHint);
    void resume();

private:
    template <typename Closure>
    void invokeOnParent(Closure c);

    // flite synthesis thread
    void runFliteSynthesis();
    int outputCallback(const cst_wave *w, int start, int size, int last,
                       struct cst_audio_streaming_info_struct *asi);
    static std::optional<TokenInformation>
    detectNewToken(const cst_wave *w, int start, int size,
                   struct cst_audio_streaming_info_struct *asi);

    // QIODevice interface
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }
    qint64 bytesAvailable() const override;

    // immutable state
    QTextToSpeechProcessorFlite *const m_parent;
    cst_voice *const m_voice; // borrowed
    const QString m_text;

    // thread
    std::shared_ptr<QThreadPool> m_threadPool = getFliteThreadPool();
    QFuture<void> m_task;

    // state
    QAudioFormat m_format;
    std::deque<char> m_audioBuffer;
    std::deque<TokenInformation> m_tokens;
    qsizetype m_currentBytePosition{}; // Position of m_audioBuffer.begin()
    qsizetype m_currentTokenIndex{};
    bool m_lastChunkReceived{};

    // pause/stop handling
    bool m_paused{};
    // NOTE: at the moment only BoundaryHint::Word is supported
    std::optional<QTextToSpeech::BoundaryHint> m_pauseRequest;
    std::optional<QTextToSpeech::BoundaryHint> m_stopRequest;

    std::optional<qint64> bytesToNextWord() const;
};

QFliteSynthesisProcess::QFliteSynthesisProcess(cst_voice *voice,
                                               QTextToSpeechProcessorFlite *parent, QString text,
                                               float pitch, float rate)
    : m_parent(parent), m_voice(voice), m_text(std::move(text))
{
    Q_ASSERT(m_voice);
    Q_ASSERT(m_parent);

    // prepare voice
    setRateForVoice(m_voice, rate);
    setPitchForVoice(m_voice, pitch);

    m_task = QtConcurrent::run(m_threadPool.get(), [this] {
        runFliteSynthesis();
    });

    open(ReadOnly | Unbuffered);
}

QFliteSynthesisProcess::~QFliteSynthesisProcess()
{
    m_task.cancel();
    m_task.waitForFinished();
}

void QFliteSynthesisProcess::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    if (m_paused)
        return;

    switch (boundaryHint) {
    case BoundaryHint::Default:
    case BoundaryHint::Immediate:
        Q_UNREACHABLE_RETURN();
        break;

    default:
        m_pauseRequest = boundaryHint;
    }
}

void QFliteSynthesisProcess::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    switch (boundaryHint) {
    case BoundaryHint::Default:
    case BoundaryHint::Immediate:
        Q_UNREACHABLE_RETURN();
        break;

    default:
        m_stopRequest = boundaryHint;
    }
}

void QFliteSynthesisProcess::resume()
{
    m_paused = false;
    m_pauseRequest = std::nullopt;
}

template <typename Closure>
void QFliteSynthesisProcess::invokeOnParent(Closure c)
{
    QMetaObject::invokeMethod(
            m_parent,
            [parent = m_parent, self = QPointer{ this }, closure = std::move(c)]() mutable {
        if (!parent->m_synthesisProcess || (parent->m_synthesisProcess.get() != self))
            return; // Another synthesis process has started

        closure(parent);
    }, Qt::QueuedConnection);
}

void QFliteSynthesisProcess::runFliteSynthesis()
{
    qCDebug(lcSpeechTtsFlite) << "QFliteSynthesisProcess() begin";

    cst_audio_streaming_info *asi = new_audio_streaming_info();
    asi->asc = [](const cst_wave *w, int start, int size, int last,
                  struct cst_audio_streaming_info_struct *asi) {
        auto *self = static_cast<QFliteSynthesisProcess *>(asi->userdata);
        return self->outputCallback(w, start, size, last, asi);
    };
    asi->userdata = (void *)this;
    feat_set(m_voice->features, "streaming_info", audio_streaming_info_val(asi));

    float secsToSpeak = flite_text_to_speech(m_text.toUtf8().constData(), m_voice, "none");

    if (secsToSpeak <= 0) {
        invokeOnParent([](QTextToSpeechProcessorFlite *parent) {
            parent->setError(
                    QTextToSpeech::ErrorReason::Input,
                    QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
        });
        return;
    };

    qCDebug(lcSpeechTtsFlite) << "QFliteSynthesisProcess() end" << secsToSpeak << "Seconds";
}

int QFliteSynthesisProcess::outputCallback(const cst_wave *w, int start, int size, int last,
                                           cst_audio_streaming_info_struct *asi)
{
    Q_ASSERT(w);

    if (start == 0) {
        invokeOnParent([this, format = getAudioFormat(*w)](QTextToSpeechProcessorFlite *parent) {
            m_format = format;
            parent->prepareAudioSink(format);
        });
    }

    QSpan fliteStream{
        w->samples + start,
        size,
    };
    QByteArray chunk{
        reinterpret_cast<const char *>(fliteStream.data()),
        fliteStream.size_bytes(),
    };

    std::optional<TokenInformation> token = detectNewToken(w, start, size, asi);

    invokeOnParent([this, chunk = std::move(chunk), token = std::move(token),
                    last](QTextToSpeechProcessorFlite *) mutable {
        m_audioBuffer.insert(m_audioBuffer.end(), chunk.begin(), chunk.end());

        if (token)
            m_tokens.push_back(std::move(*token));
        if (last)
            m_lastChunkReceived = true;

        emit QIODevice::bytesAvailable();
    });

    if (m_task.isCanceled())
        return CST_AUDIO_STREAM_STOP;
    return CST_AUDIO_STREAM_CONT;
}

qint64 QFliteSynthesisProcess::readData(char *data, qint64 maxlen)
{
    if (m_paused)
        Q_ASSERT(m_pauseRequest || m_stopRequest);

    const qint64 bytesAvailable = this->bytesAvailable();
    const qint64 bytesRequested = std::min(bytesAvailable, maxlen);
    qint64 bytesToRead = bytesRequested;

    bool atWordBoundary = false;
    if (!m_paused && (m_pauseRequest || m_stopRequest)) {
        std::optional<qsizetype> bytesToNextWord = this->bytesToNextWord();
        if (bytesToNextWord && bytesToNextWord < bytesRequested) {
            // We are at a word boundary, so we only read up to the next word.
            bytesToRead = bytesToNextWord.value();
            atWordBoundary = true;
        }
    }

    if (m_paused) {
        // feed null to sink during async operation
        std::fill_n(data, bytesToRead, 0);
    } else {
        std::copy_n(m_audioBuffer.begin(), bytesToRead, data);
        std::fill_n(data, bytesRequested - bytesToRead, 0);

        m_audioBuffer.erase(m_audioBuffer.begin(), m_audioBuffer.begin() + bytesToRead);

        m_currentBytePosition += bytesToRead;

        const std::chrono::microseconds currentTimeStamp{
            m_format.durationForBytes(m_currentBytePosition),
        };

        while (!m_tokens.empty() && m_tokens.front().startTime <= currentTimeStamp) {
            const TokenInformation &token = m_tokens.front();
            m_currentTokenIndex = m_text.indexOf(token.word, m_currentTokenIndex);
            emit m_parent->sayingWord(token.word, m_currentTokenIndex, token.word.length());
            m_tokens.pop_front();
        }
    }

    const bool stopSynthesisProcess = [&] {
        if (m_lastChunkReceived && m_audioBuffer.empty())
            return true; // end of file reached
        if (atWordBoundary && m_stopRequest == BoundaryHint::Word)
            return true; // stop at word boundary
        return false;
    }();

    if (stopSynthesisProcess) {
        m_paused = true; // we feed silence to the audio sink until the stop is processed

        invokeOnParent([](QTextToSpeechProcessorFlite *parent) {
            parent->stop(QTextToSpeech::BoundaryHint::Immediate);
        });
    } else if (atWordBoundary && m_pauseRequest == BoundaryHint::Word) {
        m_paused = true;

        invokeOnParent([](QTextToSpeechProcessorFlite *parent) {
            parent->pause(QTextToSpeech::BoundaryHint::Immediate);
        });
    }

    return bytesToRead;
}

qint64 QFliteSynthesisProcess::bytesAvailable() const
{
    return qint64(m_audioBuffer.size());
}

std::optional<qint64> QFliteSynthesisProcess::bytesToNextWord() const
{
    if (m_tokens.empty())
        return std::nullopt;

    using namespace std::chrono;

    const microseconds currentTimeStamp{
        m_format.durationForBytes(m_currentBytePosition),
    };
    const microseconds nextTokenStart{
        m_tokens.front().startTime,
    };
    return m_format.bytesForDuration((nextTokenStart - currentTimeStamp).count());
}

std::optional<QFliteSynthesisProcess::TokenInformation>
QFliteSynthesisProcess::detectNewToken(const cst_wave *w, int start, int size,
                                       cst_audio_streaming_info_struct *asi)
{
    if (!asi->item)
        asi->item = relation_head(utt_relation(asi->utt, "Token"));

    const float tokenStartTime = flite_ffeature_float(
            asi->item, "R:Token.daughter1.R:SylStructure.daughter1.daughter1.R:Segment.p.end");
    const int tokenStartSample = int(tokenStartTime * float(w->sample_rate));
    if ((tokenStartSample >= start) && (tokenStartSample < start + size)) {

        const char *token = flite_ffeature_string(asi->item, "name");
        if (!token) {
            Q_UNLIKELY_BRANCH;
            qCWarning(lcSpeechTtsFlite) << "No token found, skipping";
            return std::nullopt;
        }

        auto normalizeFeatureString = [&](const char *feature) -> const char * {
            const char *featureString = flite_ffeature_string(asi->item, feature);
            if (cst_streq("0", featureString))
                return "";
            return featureString;
        };

        auto tokenStartTimestamp = std::chrono::milliseconds(std::lround(tokenStartTime * 1'000));

        qCDebug(lcSpeechTtsFlite).nospace()
                << "Processing token start_time: " << tokenStartTimestamp << " content: \""
                << flite_ffeature_string(asi->item, "whitespace")
                << normalizeFeatureString("prepunctuation") << "'" << token << "'"
                << normalizeFeatureString("punc") << "\"";

        asi->item = item_next(asi->item);
        return TokenInformation{
            QString::fromUtf8(token),
            tokenStartTimestamp,
        };
    }
    return std::nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QTextToSpeechProcessorFlite::QTextToSpeechProcessorFlite(QAudioDevice audioDevice)
    : m_audioDevice(std::move(audioDevice))
{
    init();
}

QTextToSpeechProcessorFlite::~QTextToSpeechProcessorFlite()
{
    m_synthesisProcess.reset();
    for (const VoiceInfo &voice : std::as_const(m_voices))
        voice.unregister_func(voice.vox);
}

const QList<QTextToSpeechProcessorFlite::VoiceInfo> &QTextToSpeechProcessorFlite::voices() const
{
    return m_voices;
}

typedef cst_voice*(*registerFnType)();
typedef void(*unregisterFnType)(cst_voice *);

bool QTextToSpeechProcessorFlite::init()
{
    flite_init();

    const QLocale locale(QLocale::English, QLocale::UnitedStates);
    // ### FIXME: hardcode for now, the only voice files we know about are for en_US
    // We could source the language and perhaps the list of voices we want to load
    // (hardcoded below) from an environment variable.
    const QString langCode(u"us"_s);
    const QString libPrefix(u"flite_cmu_%1_%2.so.1"_s);
    const QString registerPrefix(u"register_cmu_%1_%2"_s);
    const QString unregisterPrefix(u"unregister_cmu_%1_%2"_s);

    for (const auto &voice : fliteAvailableVoices(libPrefix, langCode)) {
        QLibrary library(libPrefix.arg(langCode, voice));
        if (!library.load()) {
            qWarning("Voice library could not be loaded: %s", qPrintable(library.fileName()));
            continue;
        }
        auto registerFn = reinterpret_cast<registerFnType>(library.resolve(
            registerPrefix.arg(langCode, voice).toLatin1().constData()));
        auto unregisterFn = reinterpret_cast<unregisterFnType>(library.resolve(
            unregisterPrefix.arg(langCode, voice).toLatin1().constData()));
        if (registerFn && unregisterFn) {
            const int id = int(m_voices.count());
            m_voices.append(VoiceInfo{
                id,
                registerFn(),
                unregisterFn,
                voice,
                locale.name(),
                QVoice::Male,
                QVoice::Adult
            });
        } else {
            library.unload();
        }
    }

    return !m_voices.isEmpty();
}

void QTextToSpeechProcessorFlite::prepareAudioSink(QAudioFormat format)
{
    qCDebug(lcSpeechTtsFlite) << "QTextToSpeechProcessorFlite::prepareAudioSink" << format;

    m_audioSink = std::make_unique<QAudioSink>(m_audioDevice, format);
    m_audioSink->setVolume(m_volume);
    m_audioSink->setBufferSize(format.bytesForDuration(std::chrono::microseconds(100ms).count()));

    // LATER: use public API (compare QTBUG-138378)
    QPlatformAudioSink *platformAudioSink = QPlatformAudioSink::get(*m_audioSink);
    if (platformAudioSink)
        platformAudioSink->setRole(QPlatformAudioSink::AudioEndpointRole::Accessibility);

    QObject::connect(m_audioSink.get(), &QAudioSink::stateChanged, m_audioSink.get(),
                     [&](QAudio::State state) {
        if (state == QAudio::StoppedState && m_audioSink->error() != QAudio::NoError) {
            setError(QTextToSpeech::ErrorReason::Playback,
                     QCoreApplication::translate("QTextToSpeech", "Audio IO."));
        }
    });

    m_audioSink->start(m_synthesisProcess.get());
    if (m_audioSink->error() != QAudio::NoError) {
        setError(QTextToSpeech::ErrorReason::Playback,
                 QCoreApplication::translate("QTextToSpeech", "Audio Open error: %1")
                         .arg(m_audioSink->error()));
        return;
    }
}

void QTextToSpeechProcessorFlite::setError(QTextToSpeech::ErrorReason err, const QString &errorString)
{
    if (err == QTextToSpeech::ErrorReason::NoError)
        return;

    m_audioSink.reset();
    if (m_synthesisProcess)
        m_synthesisProcess.reset();

    qCDebug(lcSpeechTtsFlite) << "Error" << err << errorString;
    updateState(QTextToSpeech::Error);
    emit errorOccurred(err, errorString);
}

void QTextToSpeechProcessorFlite::updateState(QTextToSpeech::State state)
{
    if (state == m_state)
        return;
    m_state = state;
    qCDebug(lcSpeechTtsFlite) << "State changed to" << state;
    emit stateChanged(state);
}

// Check voice validity
bool QTextToSpeechProcessorFlite::checkVoice(int voiceId)
{
    if (voiceId >= 0 && voiceId < m_voices.size())
        return true;

    setError(QTextToSpeech::ErrorReason::Configuration,
             QCoreApplication::translate("QTextToSpeech", "Invalid voiceId %1.").arg(voiceId));
    return false;
}


// Stop current and cancel subsequent utterances
void QTextToSpeechProcessorFlite::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    using BoundaryHint = QTextToSpeech::BoundaryHint;

    switch (m_state) {
    case QTextToSpeech::Speaking:
    case QTextToSpeech::Paused: {
        switch (boundaryHint) {
        case BoundaryHint::Sentence: {
            qCDebug(lcSpeechTtsFlite)
                    << "Stopping after sentence not implemented. Stopping after next word";
            return stop(BoundaryHint::Word);
        }
        case BoundaryHint::Utterance:
            Q_UNREACHABLE_RETURN(); // handled by QTextToSpeech
        case BoundaryHint::Word: {
            m_synthesisProcess->stop(BoundaryHint::Word);
            return;
        }
        default: {
            if (m_audioSink) {
                m_audioSink->reset();
                m_audioSink.reset();
            }
            m_synthesisProcess.reset();
            updateState(QTextToSpeech::Ready);
            break;
        }
        }
    }

    case QTextToSpeech::Synthesizing:
        return; // we cannot stop a synthesis process, it will stop automatically

    case QTextToSpeech::Error: {
        m_synthesisProcess.reset();

        updateState(QTextToSpeech::Ready);
        return;
    }
    case QTextToSpeech::Ready:
        break;

    default:
        Q_UNREACHABLE();
    }
}

void QTextToSpeechProcessorFlite::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    using BoundaryHint = QTextToSpeech::BoundaryHint;

    if (m_state == QTextToSpeech::Speaking) {
        switch (boundaryHint) {
        case BoundaryHint::Sentence: {
            qCDebug(lcSpeechTtsFlite)
                    << "Pausing after sentence not implemented. Pausing after next word";
            return pause(BoundaryHint::Word);
        }
        case BoundaryHint::Utterance:
            Q_UNREACHABLE_RETURN(); // handled by QTextToSpeech

        case BoundaryHint::Word: {
            m_synthesisProcess->pause(BoundaryHint::Word);
            return;
        }

        default:
            if (m_audioSink)
                m_audioSink->suspend();
            updateState(QTextToSpeech::Paused);
        }
    }
}

void QTextToSpeechProcessorFlite::resume()
{
    if (m_synthesisProcess)
        m_synthesisProcess->resume();

    if (m_state == QTextToSpeech::Paused) {
        if (m_audioSink && m_synthesisProcess)
            m_audioSink->resume();
        updateState(QTextToSpeech::Speaking);
    }
}

void QTextToSpeechProcessorFlite::say(const QString &text, int voiceId, double pitch, double rate, double volume)
{
    if (text.isEmpty())
        return;

    if (!checkVoice(voiceId))
        return;

    switch (m_state) {
    case QTextToSpeech::Speaking:
    case QTextToSpeech::Paused:
        stop(QTextToSpeech::BoundaryHint::Immediate);
        break;

    case QTextToSpeech::Synthesizing:
        return; // we cannot synthesize and speak at the same time

    case QTextToSpeech::Ready:
    case QTextToSpeech::Error:
        break;

    default:
        Q_UNREACHABLE();
    }

    const VoiceInfo &voiceInfo = m_voices.at(voiceId);
    m_volume = float(volume);
    updateState(QTextToSpeech::Speaking);
    m_synthesisProcess = std::make_unique<QFliteSynthesisProcess>(voiceInfo.vox, this, text,
                                                                  float(pitch), float(rate));
}

void QTextToSpeechProcessorFlite::synthesize(const QString &text, int voiceId, double pitch, double rate, double volume)
{
    if (text.isEmpty())
        return;

    if (!checkVoice(voiceId))
        return;

    switch (m_state) {
    case QTextToSpeech::Speaking:
    case QTextToSpeech::Paused:
        return; // we cannot synthesize and speak at the same time

    case QTextToSpeech::Synthesizing:
    case QTextToSpeech::Ready:
    case QTextToSpeech::Error:
        break;

    default:
        Q_UNREACHABLE();
    }

    m_synthesisFormat = std::nullopt;
    m_volume = float(volume);

    qCDebug(lcSpeechTtsFlite) << "processText() begin";

    const VoiceInfo &voiceInfo = m_voices.at(voiceId);
    cst_voice *voice = voiceInfo.vox;
    cst_audio_streaming_info *asi = new_audio_streaming_info();

    asi->asc = [](const cst_wave *w, int start, int size, int /*last*/,
                  cst_audio_streaming_info *asi) -> int {
        auto *self = static_cast<QTextToSpeechProcessorFlite *>(asi->userdata);

        if (!self->m_synthesisFormat) {
            QAudioFormat format = getAudioFormat(*w);
            if (!format.isValid())
                return CST_AUDIO_STREAM_STOP;
            self->m_synthesisFormat = format;
        }

        const qsizetype bytesToWrite = size * self->m_synthesisFormat->bytesPerSample();
        QByteArray chunk(reinterpret_cast<const char *>(w->samples + start), bytesToWrite);

        QAudioHelperInternal::applyVolume(self->m_volume, *self->m_synthesisFormat,
                                          as_bytes(QSpan{ chunk }),
                                          as_writable_bytes(QSpan{ chunk }));

        emit self->synthesized(*self->m_synthesisFormat, chunk);
        return CST_AUDIO_STREAM_CONT;
    };

    asi->userdata = (void *)this;
    feat_set(voice->features, "streaming_info", audio_streaming_info_val(asi));
    setRateForVoice(voice, float(rate));
    setPitchForVoice(voice, float(pitch));

    updateState(QTextToSpeech::Synthesizing);

    float secsToSpeak = flite_text_to_speech(text.toUtf8().constData(), voice, "none");

    if (secsToSpeak <= 0) {
        setError(QTextToSpeech::ErrorReason::Input,
                 QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
        return;
    }

    qCDebug(lcSpeechTtsFlite) << "processText() end" << secsToSpeak << "Seconds";
    m_synthesisFormat = std::nullopt;
    updateState(QTextToSpeech::Ready);
}

QT_END_NAMESPACE
