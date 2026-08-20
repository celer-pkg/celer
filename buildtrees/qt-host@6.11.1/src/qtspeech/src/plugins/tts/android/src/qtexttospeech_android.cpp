// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_android.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qoperatingsystemversion.h>

QT_BEGIN_NAMESPACE

static jclass g_qtSpeechClass = 0;

typedef QMap<jlong, QTextToSpeechEngineAndroid *> TextToSpeechMap;
Q_GLOBAL_STATIC(TextToSpeechMap, textToSpeechMap)

Q_DECLARE_JNI_CLASS(Locale, "java/util/Locale")

static void notifyError(JNIEnv *env, jobject thiz, jlong id, jlong reason)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    QMetaObject::invokeMethod(tts, "processNotifyError", Qt::AutoConnection,
                              Q_ARG(int, reason));
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyError)

static void notifyReady(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    QMetaObject::invokeMethod(tts, "processNotifyReady", Qt::AutoConnection);
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyReady)

static void notifySpeaking(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    QMetaObject::invokeMethod(tts, "processNotifySpeaking", Qt::AutoConnection);
}
Q_DECLARE_JNI_NATIVE_METHOD(notifySpeaking)

static void notifyRangeStart(JNIEnv *env, jobject thiz, jlong id, jint start, jint end, jint frame)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    QMetaObject::invokeMethod(tts, "processNotifyRangeStart", Qt::AutoConnection,
        Q_ARG(int, start), Q_ARG(int, end), Q_ARG(int, frame));
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyRangeStart)

static void notifyBeginSynthesis(JNIEnv *env, jobject thiz, jlong id, int sampleRateInHz, int audioFormat, int channelCount)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    QAudioFormat format;
    format.setSampleRate(sampleRateInHz);
    format.setSampleFormat(QAudioFormat::SampleFormat(audioFormat));
    format.setChannelCount(channelCount);

    QMetaObject::invokeMethod(tts, "processNotifyBeginSynthesis", Qt::AutoConnection,
        Q_ARG(QAudioFormat, format));
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyBeginSynthesis)

static void notifyAudioAvailable(JNIEnv *env, jobject thiz, jlong id, jbyteArray bytes)
{
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    const auto sz = env->GetArrayLength(bytes);
    QByteArray byteArray(sz, Qt::Initialization::Uninitialized);
    env->GetByteArrayRegion(bytes, 0, sz, reinterpret_cast<jbyte *>(byteArray.data()));

    QMetaObject::invokeMethod(tts, "processNotifyAudioAvailable", Qt::AutoConnection,
        Q_ARG(QByteArray, byteArray));
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyAudioAvailable)

static void notifyEndSynthesis(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechEngineAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    // Queued so that pending processNotifyAudioAvailable
    // invocations get processed first.
    QMetaObject::invokeMethod(tts, "processNotifyReady", Qt::QueuedConnection);
}
Q_DECLARE_JNI_NATIVE_METHOD(notifyEndSynthesis)


extern "C" Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void */*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    typedef union {
        JNIEnv *nativeEnvironment;
        void *venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;

    QJniEnvironment jniEnv;
    jclass clazz = jniEnv.findClass<QtJniTypes::QtTextToSpeech>();

    if (clazz) {
        g_qtSpeechClass = static_cast<jclass>(jniEnv->NewGlobalRef(clazz));
        if (!jniEnv.registerNativeMethods(clazz, {
            Q_JNI_NATIVE_METHOD(notifyError),
            Q_JNI_NATIVE_METHOD(notifyReady),
            Q_JNI_NATIVE_METHOD(notifySpeaking),
            Q_JNI_NATIVE_METHOD(notifyRangeStart),
            Q_JNI_NATIVE_METHOD(notifyBeginSynthesis),
            Q_JNI_NATIVE_METHOD(notifyAudioAvailable),
            Q_JNI_NATIVE_METHOD(notifyEndSynthesis),
        })) {
            return JNI_ERR;
        }
    }

    return JNI_VERSION_1_6;
}

QTextToSpeechEngineAndroid::QTextToSpeechEngineAndroid(const QVariantMap &parameters, QObject *parent)
    : QTextToSpeechEngine(parent)
{
    Q_ASSERT(g_qtSpeechClass);

    const QString engine = parameters.value("androidEngine").toString();

    const jlong id = reinterpret_cast<jlong>(this);
    m_speech = QJniObject::construct<QtJniTypes::QtTextToSpeech>(QNativeInterface::QAndroidApplication::context(),
                                                                 id, QJniObject::fromString(engine).object<jstring>());
    (*textToSpeechMap)[id] = this;
}

QTextToSpeechEngineAndroid::~QTextToSpeechEngineAndroid()
{
    textToSpeechMap->remove(reinterpret_cast<jlong>(this));
    m_speech.callMethod<void>("shutdown");
}

QTextToSpeech::Capabilities QTextToSpeechEngineAndroid::capabilities() const
{
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::AndroidNougat)
        return QTextToSpeech::Capability::Speak;
    // the default uses the values from the plugin's meta data
    return QTextToSpeechEngine::capabilities();
}

void QTextToSpeechEngineAndroid::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state == QTextToSpeech::Speaking)
        stop(QTextToSpeech::BoundaryHint::Default);

    m_text = text;
    m_speech.callMethod<void>("say", QJniObject::fromString(m_text).object<jstring>());
}

void QTextToSpeechEngineAndroid::synthesize(const QString &text)
{
    if (text.isEmpty())
        return;

    m_errorReason = QTextToSpeech::ErrorReason::NoError;
    m_text = text;
    m_speech.callMethod<int>("synthesize", QJniObject::fromString(m_text).object<jstring>());
}

void QTextToSpeechEngineAndroid::processNotifyBeginSynthesis(const QAudioFormat &format)
{
    m_format = format;
    setState(QTextToSpeech::Synthesizing);
}

void QTextToSpeechEngineAndroid::processNotifyAudioAvailable(const QByteArray &bytes)
{
    Q_ASSERT(m_format.isValid());
    emit synthesized(m_format, bytes);
}

QTextToSpeech::State QTextToSpeechEngineAndroid::state() const
{
    return m_state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineAndroid::errorReason() const
{
    return m_errorReason;
}

QString QTextToSpeechEngineAndroid::errorString() const
{
    return m_errorString;
}

void QTextToSpeechEngineAndroid::setState(QTextToSpeech::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged(m_state);
    if (m_state == QTextToSpeech::Error) {
        emit errorOccurred(m_errorReason, m_errorString);
    } else {
        m_errorReason = QTextToSpeech::ErrorReason::NoError;
        m_errorString.clear();
    }
}

void QTextToSpeechEngineAndroid::setError(QTextToSpeech::ErrorReason reason, const QString &string)
{
    m_errorReason = reason;
    m_errorString = string;
    if (reason != QTextToSpeech::ErrorReason::NoError)
        return;
    if (m_state != QTextToSpeech::Error) {
        m_state = QTextToSpeech::Error;
        emit stateChanged(m_state);
    }
    emit errorOccurred(m_errorReason, m_errorString);
}

void QTextToSpeechEngineAndroid::processNotifyReady()
{
    if (m_state == QTextToSpeech::Synthesizing)
        m_format = {};
    if (m_state != QTextToSpeech::Paused)
        setState(QTextToSpeech::Ready);
}

void QTextToSpeechEngineAndroid::processNotifyError(int errorReason)
{
    switch (errorReason) {
    case 1:
        setError(QTextToSpeech::ErrorReason::Initialization,
                 QCoreApplication::translate("QTextToSpeech",
                    "Failed to initialize text-to-speech engine."));
        break;
    case 2:
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech",
                    "Could not apply text-to-speech parameters."));
        break;
    case 3:
        setError(QTextToSpeech::ErrorReason::Input,
                 QCoreApplication::translate("QTextToSpeech",
                    "Speech synthesizing failure."));
        break;
    case 4:
        setError(QTextToSpeech::ErrorReason::Playback,
                 QCoreApplication::translate("QTextToSpeech",
                    "Failure while rendering speech to audio device."));
        break;
    default:
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "Unknown error."));
        break;
    }
}

void QTextToSpeechEngineAndroid::processNotifySpeaking()
{
    setState(QTextToSpeech::Speaking);
}

void QTextToSpeechEngineAndroid::processNotifyRangeStart(int start, int end, int frame)
{
    Q_UNUSED(frame);
    const int length = end - start;
    emit sayingWord(m_text.sliced(start, length), start,length);
}

void QTextToSpeechEngineAndroid::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (m_state == QTextToSpeech::Ready)
        return;

    m_speech.callMethod<void>("stop");
    setState(QTextToSpeech::Ready);
    m_format = {};
}

void QTextToSpeechEngineAndroid::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (m_state == QTextToSpeech::Paused)
        return;

    m_speech.callMethod<void>("stop");
    setState(QTextToSpeech::Paused);
}

void QTextToSpeechEngineAndroid::resume()
{
    if (m_state != QTextToSpeech::Paused)
        return;

    say(m_text);
}

// Android API's pitch is from (0.0, 2.0[ with 1.0 being normal. 0.0 is
// not included, so we have to scale negative Qt pitches from 0.1 to 1.0
double QTextToSpeechEngineAndroid::pitch() const
{
    jfloat pitch = m_speech.callMethod<jfloat>("pitch");
    if (pitch < 1.0f)
        pitch = (pitch - 1.0f) / 0.9f;
    else
        pitch -= 1.0f;
    return double(pitch);
}

bool QTextToSpeechEngineAndroid::setPitch(double pitch)
{
    if (pitch < 0)
        pitch = (pitch * 0.9f) + 1.0f;
    else
        pitch += 1.0f;

    // 0 == SUCCESS
    return m_speech.callMethod<int>("setPitch", float(pitch)) == 0;
}

// Android API's rate is from [0.5, 2.0[, with 1.0 being normal.
double QTextToSpeechEngineAndroid::rate() const
{
    jfloat rate = m_speech.callMethod<jfloat>("rate");
    if (rate < 1.0f)
        rate = (rate - 1.0f) * 2.0f;
    else
        rate -= 1.0f;
    return double(rate);
}

bool QTextToSpeechEngineAndroid::setRate(double rate)
{
    rate = 1.0 + (rate >= 0 ? rate : (rate * 0.5));
    // 0 == SUCCESS
    return m_speech.callMethod<int>("setRate", float(rate)) == 0;
}

double QTextToSpeechEngineAndroid::volume() const
{
    jfloat volume = m_speech.callMethod<jfloat>("volume");
    return volume;
}

bool QTextToSpeechEngineAndroid::setVolume(double volume)
{
    // 0 == SUCCESS
    return m_speech.callMethod<jint>("setVolume", float(volume)) == 0;
}

QList<QLocale> QTextToSpeechEngineAndroid::availableLocales() const
{
    auto locales = m_speech.callObjectMethod("getAvailableLocales", "()Ljava/util/List;");
    int count = locales.callMethod<jint>("size");
    QList<QLocale> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        auto locale = locales.callMethod<jobject>("get", i);
        if (locale.isValid()) {
            auto localeLanguage = locale.callMethod<jstring>("getLanguage").toString();
            auto localeCountry = locale.callMethod<jstring>("getCountry").toString();
            if (!localeCountry.isEmpty())
                localeLanguage += QString("_%1").arg(localeCountry).toUpper();
            result << QLocale(localeLanguage);
        }
    }
    return result;
}

bool QTextToSpeechEngineAndroid::setLocale(const QLocale &locale)
{
    QStringList parts = locale.name().split('_');

    if (parts.length() != 2)
        return false;

    QString languageCode = parts.at(0);
    QString countryCode = parts.at(1);

    QJniObject jLocale("java/util/Locale", QJniObject::fromString(languageCode).object<jstring>(),
                                           QJniObject::fromString(countryCode).object<jstring>());

    if (!m_speech.callMethod<jboolean>("setLocale", jLocale.object<QtJniTypes::Locale>())) {
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "No voice available for locale %1")
                    .arg(locale.bcp47Name()));
        return false;
    }
    return true;
}

QLocale QTextToSpeechEngineAndroid::locale() const
{
    auto locale = m_speech.callMethod<QtJniTypes::Locale>("getLocale");
    if (locale.isValid()) {
        auto localeLanguage = locale.callMethod<jstring>("getLanguage").toString();
        auto localeCountry = locale.callMethod<jstring>("getCountry").toString();
        if (!localeCountry.isEmpty())
            localeLanguage += QString("_%1").arg(localeCountry).toUpper();
        return QLocale(localeLanguage);
    }
    return QLocale();
}

QVoice QTextToSpeechEngineAndroid::javaVoiceObjectToQVoice(QJniObject &obj) const
{
    auto voiceName = obj.callMethod<jstring>("getName").toString();
    QVoice::Gender gender;
    if (voiceName.contains(QStringLiteral("#male"))) {
        gender = QVoice::Male;
    } else if (voiceName.contains(QStringLiteral("#female"))) {
        gender = QVoice::Female;
    } else {
        gender = QVoice::Unknown;
    }
    QJniObject locale = obj.callMethod<QtJniTypes::Locale>("getLocale");
    QLocale qlocale;
    if (locale.isValid()) {
        auto localeLanguage = locale.callMethod<jstring>("getLanguage").toString();
        auto localeCountry = locale.callMethod<jstring>("getCountry").toString();
        if (!localeCountry.isEmpty())
            localeLanguage += QString("_%1").arg(localeCountry).toUpper();
        qlocale = QLocale(localeLanguage);
    }
    return createVoice(voiceName, qlocale, gender, QVoice::Other, voiceName);
}

QList<QVoice> QTextToSpeechEngineAndroid::availableVoices() const
{
    auto voices = m_speech.callObjectMethod("getAvailableVoices", "()Ljava/util/List;");
    int count = voices.callMethod<jint>("size");
    const QLocale ttsLocale = locale();
    QList<QVoice> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        auto jvoice = voices.callMethod<jobject>("get", i);
        const QVoice voice = javaVoiceObjectToQVoice(jvoice);
        if (voice.locale() == ttsLocale)
            result << voice;
    }
    return result;
}

bool QTextToSpeechEngineAndroid::setVoice(const QVoice &voice)
{
    const QString id = voiceData(voice).toString();
    if (!m_speech.callMethod<jboolean>("setVoice", QJniObject::fromString(id).object<jstring>())) {
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "Could not set voice"));
        return false;
    }
    return true;
}

QVoice QTextToSpeechEngineAndroid::voice() const
{
    auto voice = m_speech.callMethod<jobject>("getVoice");
    if (voice.isValid())
        return javaVoiceObjectToQVoice(voice);
    return QVoice();
}

QT_END_NAMESPACE
