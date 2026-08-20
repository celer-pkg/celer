// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_sapi.h"

#include <windows.h>
#ifdef Q_CC_MSVC
#pragma warning(disable : 4996)
#endif
#include <sapi.h>
#include <sphelper.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/qvarlengtharray.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

#ifndef SPERR_NO_MORE_ITEMS
# define SPERR_NO_MORE_ITEMS MAKE_SAPI_ERROR(0x039)
#endif

#ifdef Q_CC_MINGW // from sphelper.h

static const GUID CLSD_SpVoice = {0x96749377, 0x3391, 0x11d2,{0x9e, 0xe3, 0x0, 0xc0, 0x4f, 0x79, 0x73, 0x96}};
const GUID SPDFID_WaveFormatEx = {0xC31ADBAE, 0x527F, 0x4ff5,{0xA2, 0x30, 0xF6, 0x2B, 0xB6, 0x1F, 0xF7, 0x0C}};

static inline HRESULT SpGetTokenFromId(const WCHAR *pszTokenId, ISpObjectToken **cpToken, BOOL fCreateIfNotExist = FALSE)
{
    LPUNKNOWN pUnkOuter = nullptr;
    HRESULT hr = ::CoCreateInstance(CLSID_SpObjectToken, pUnkOuter, CLSCTX_ALL,
                                    __uuidof(ISpObjectToken), reinterpret_cast<void **>(cpToken));
    if (SUCCEEDED(hr))
        hr = (*cpToken)->SetId(NULL, pszTokenId, fCreateIfNotExist);
    return hr;
}

static inline HRESULT SpCreateNewToken(const WCHAR *pszTokenId, ISpObjectToken **ppToken)
{
    // Forcefully create the token
    return SpGetTokenFromId(pszTokenId, ppToken, TRUE);
}

inline void SpClearEvent(SPEVENT *pe)
{
    switch (pe->elParamType) {
    case SPET_LPARAM_IS_TOKEN:
    case SPET_LPARAM_IS_OBJECT:
        reinterpret_cast<IUnknown *>(pe->lParam)->Release();
        break;
    case SPET_LPARAM_IS_POINTER:
    case SPET_LPARAM_IS_STRING:
        CoTaskMemFree(reinterpret_cast<void *>(pe->lParam));
        break;
    case SPET_LPARAM_IS_UNDEFINED:
        break;
    }
}
#endif // Q_CC_MINGW

QTextToSpeechEngineSapi::QTextToSpeechEngineSapi(const QVariantMap &, QObject *)
{
    if (FAILED(::CoInitialize(NULL))) {
        qWarning() << "Init of COM failed";
        return;
    }

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&m_voice);
    if (!SUCCEEDED(hr)) {
        setError(QTextToSpeech::ErrorReason::Initialization,
                 QCoreApplication::translate("QTextToSpeech",
                                             "Could not initialize text-to-speech engine."));
        return;
    }

    m_voice->SetInterest(SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS);
    m_voice->SetNotifyCallbackInterface(this, 0, 0);
    updateVoices();
    if (m_voices.isEmpty()) {
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "No voices available."));
    } else {
        m_state = QTextToSpeech::Ready;
        m_errorReason = QTextToSpeech::ErrorReason::NoError;
    }
}

QTextToSpeechEngineSapi::~QTextToSpeechEngineSapi()
{
    if (m_voice)
        m_voice->Release();
    CoUninitialize();
}

bool QTextToSpeechEngineSapi::isSpeaking() const
{
    SPVOICESTATUS eventStatus;
    m_voice->GetStatus(&eventStatus, NULL);
    return eventStatus.dwRunningState == SPRS_IS_SPEAKING;
}

void QTextToSpeechEngineSapi::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QTextToSpeech::Ready)
        stop(QTextToSpeech::BoundaryHint::Default);

    currentText = text;
    const QString prefix = u"<pitch absmiddle=\"%1\"/>"_s.arg(m_pitch * 10);
    textOffset = prefix.length();
    currentText.prepend(prefix);

    HRESULT hr = m_voice->Speak(currentText.toStdWString().data(), SPF_ASYNC, NULL);
    if (!SUCCEEDED(hr))
        setError(QTextToSpeech::ErrorReason::Input,
                 QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
}

void QTextToSpeechEngineSapi::synthesize(const QString &text)
{
    class OutputStream : public ISpStreamFormat
    {
        ULONG m_ref = 1;
        qint64 m_pos = 0;
        qint64 m_length = 0;
        QTextToSpeechEngineSapi *m_engine = nullptr;
        QAudioFormat m_format;

    public:
        OutputStream(QTextToSpeechEngineSapi *engine)
            : m_engine(engine)
        {
            m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
            m_format.setSampleRate(16000);
            m_format.setSampleFormat(QAudioFormat::Int16);
        }
        virtual ~OutputStream() = default;

        // IUnknown
        ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
        ULONG STDMETHODCALLTYPE Release() override {
            if (!--m_ref) {
                delete this;
                return 0;
            }
            return m_ref;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override
        {
            if (!ppvInterface)
                return E_POINTER;

            if (riid == __uuidof(IUnknown)) {
                *ppvInterface = static_cast<IUnknown*>(this);
            } else if (riid == __uuidof(IStream)) {
                *ppvInterface = static_cast<IStream *>(this);
            } else if (riid == __uuidof(ISpStreamFormat)) {
                *ppvInterface = static_cast<ISpStreamFormat *>(this);
            } else {
                *ppvInterface = nullptr;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        // IStream
        HRESULT STDMETHODCALLTYPE Read(void *,ULONG,ULONG *) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE Write(const void *pv,ULONG cb,ULONG *pcbWritten) override
        {
            emit m_engine->synthesized(m_format, QByteArray(static_cast<const char *>(pv), cb));
            *pcbWritten = cb;
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) override
        {
            qint64 move = dlibMove.QuadPart;
            switch (dwOrigin) {
            case STREAM_SEEK_SET:
                m_pos = move;
                break;
            case STREAM_SEEK_CUR:
                m_pos += move;
                break;
            case STREAM_SEEK_END:
                m_pos = m_length + move;
                break;
            }
            (*plibNewPosition).QuadPart = m_pos;
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE CopyTo(IStream *,ULARGE_INTEGER,ULARGE_INTEGER *,ULARGE_INTEGER *) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE Revert(void) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE Stat(STATSTG *,DWORD) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE Clone(IStream **) override { return E_NOTIMPL; }

        // ISpStreamFormat
        HRESULT STDMETHODCALLTYPE GetFormat(GUID *pguidFormatId,WAVEFORMATEX **ppCoMemWaveFormatEx) override
        {
            *pguidFormatId = SPDFID_WaveFormatEx;
            WAVEFORMATEX *format = static_cast<WAVEFORMATEX *>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));
            format->wFormatTag = WAVE_FORMAT_PCM;
            format->nChannels = m_format.channelCount();
            format->nSamplesPerSec = m_format.sampleRate();
            format->wBitsPerSample = m_format.bytesPerSample() * 8;
            format->nBlockAlign = format->nChannels * format->wBitsPerSample / 8;
            format->nAvgBytesPerSec = format->nSamplesPerSec * format->nBlockAlign;
            format->cbSize = 0; // amount of extra format information

            *ppCoMemWaveFormatEx = format;
            return S_OK;
        }
    };

    if (text.isEmpty())
        return;

    currentText = text;
    const QString prefix = u"<pitch absmiddle=\"%1\"/>"_s.arg(m_pitch * 10);
    textOffset = prefix.length();
    currentText.prepend(prefix);

    OutputStream *outputStream = new OutputStream(this);
    m_voice->SetOutput(outputStream, false);
    HRESULT hr = m_voice->Speak(currentText.toStdWString().data(), SPF_ASYNC, NULL);
    if (!SUCCEEDED(hr))
        setError(QTextToSpeech::ErrorReason::Input,
                 QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure."));
}

void QTextToSpeechEngineSapi::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (m_state == QTextToSpeech::Paused || m_pauseRequested)
        resume();
    m_voice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);
    currentText.clear();
}

void QTextToSpeechEngineSapi::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    if (!isSpeaking())
        return;

    // SAPI voices count calls to Pause() and require an equal
    // number of calls to Resume(); we don't want that, so don't
    // call either more than once.
    if (!m_pauseRequested && m_state != QTextToSpeech::Paused) {
        m_pauseRequested = true;
        m_voice->Pause();
    }
}

void QTextToSpeechEngineSapi::resume()
{
    if (m_pauseRequested || m_state == QTextToSpeech::Paused) {
        m_pauseRequested = false;
        m_voice->Resume();
    }
}

bool QTextToSpeechEngineSapi::setPitch(double pitch)
{
    m_pitch = pitch;
    return true;
}

double QTextToSpeechEngineSapi::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineSapi::setRate(double rate)
{
    // -10 to 10
    m_voice->SetRate(long(rate*10));
    return true;
}

double QTextToSpeechEngineSapi::rate() const
{
    long rateValue;
    if (m_voice->GetRate(&rateValue) == S_OK)
        return rateValue / 10.0;
    return -1;
}

bool QTextToSpeechEngineSapi::setVolume(double volume)
{
    // 0 to 1
    m_voice->SetVolume(volume * 100);
    return true;
}

double QTextToSpeechEngineSapi::volume() const
{
    USHORT baseVolume;
    if (m_voice->GetVolume(&baseVolume) == S_OK)
        return baseVolume / 100.0;
    return 0.0;
}

QString QTextToSpeechEngineSapi::voiceId(ISpObjectToken *speechToken) const
{
    HRESULT hr = S_OK;
    LPWSTR vId = nullptr;
    hr = speechToken->GetId(&vId);
    if (FAILED(hr)) {
        qWarning() << "ISpObjectToken::GetId failed";
        return QString();
    }
    return QString::fromWCharArray(vId);
}

QMap<QString, QString> QTextToSpeechEngineSapi::voiceAttributes(ISpObjectToken *speechToken) const
{
    HRESULT hr = S_OK;
    QMap<QString, QString> result;

    ISpDataKey *pAttrKey = nullptr;
    hr = speechToken->OpenKey(L"Attributes", &pAttrKey);
    if (FAILED(hr)) {
        qWarning() << "ISpObjectToken::OpenKeys failed";
        return result;
    }

    // enumerate values
    for (ULONG v = 0; ; v++) {
        LPWSTR val = nullptr;
        hr = pAttrKey->EnumValues(v, &val);
        if (SPERR_NO_MORE_ITEMS == hr) {
            // done
            break;
        } else if (FAILED(hr)) {
            qWarning() << "ISpDataKey::EnumValues failed";
            continue;
        }

        // how do we know whether it's a string or a DWORD?
        LPWSTR data = nullptr;
        hr = pAttrKey->GetStringValue(val, &data);
        if (FAILED(hr)) {
            qWarning() << "ISpDataKey::GetStringValue failed";
            continue;
        }

        if (0 != wcscmp(val, L""))
            result[QString::fromWCharArray(val)] = QString::fromWCharArray(data);

        // FIXME: Do we need to free the memory here?
        CoTaskMemFree(val);
        CoTaskMemFree(data);
    }

    return result;
}

QLocale QTextToSpeechEngineSapi::lcidToLocale(const QString &lcid) const
{
    bool ok;
    LCID locale = lcid.toInt(&ok, 16);
    if (!ok) {
        qWarning() << "Could not convert language attribute to LCID";
        return QLocale();
    }

    // Windows Vista+
    WCHAR name[LOCALE_NAME_MAX_LENGTH] = {};
    if (LCIDToLocaleName(locale, name, LOCALE_NAME_MAX_LENGTH, 0))
        return QLocale(QString::fromWCharArray(name));

    // Fallback to GetLocaleInfoW
    const int nchars = GetLocaleInfoW(locale, LOCALE_SISO639LANGNAME, NULL, 0);
    QVarLengthArray<wchar_t, 12> languageCode(nchars);
    GetLocaleInfoW(locale, LOCALE_SISO639LANGNAME, languageCode.data(), nchars);

    // Use country code when available
    const int countryCodeSize = GetLocaleInfoW(locale, LOCALE_SISO3166CTRYNAME, NULL, 0);
    if (countryCodeSize > 0) {
        QVarLengthArray<wchar_t, 12> countryCode(countryCodeSize);
        GetLocaleInfoW(locale, LOCALE_SISO3166CTRYNAME, countryCode.data(), countryCodeSize);
        return QLocale(u"%1_%2"_s.arg(QString::fromWCharArray(languageCode.data()),
                                      QString::fromWCharArray(countryCode.data())));
    }

    return QLocale(QString::fromWCharArray(languageCode.data()));
}

QVoice::Age QTextToSpeechEngineSapi::toVoiceAge(const QString &age) const
{
    // See https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee431801(v=vs.85)#61-category-voices
    if (age == u"Child")
        return QVoice::Child;
    if (age == u"Teen")
        return QVoice::Teenager;
    if (age == u"Adult")
        return QVoice::Adult;
    if (age == u"Senior")
        return QVoice::Senior;
    return QVoice::Other;
}

void QTextToSpeechEngineSapi::updateVoices()
{
    HRESULT hr = S_OK;
    ISpObjectToken *cpVoiceToken = nullptr;
    IEnumSpObjectTokens *cpEnum = nullptr;
    ULONG ulCount = 0;

    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
    if (SUCCEEDED(hr))
        hr = cpEnum->GetCount(&ulCount);

    // Loop through all voices
    while (SUCCEEDED(hr) && ulCount--) {
        if (cpVoiceToken) {
            cpVoiceToken->Release();
            cpVoiceToken = nullptr;
        }
        hr = cpEnum->Next(1, &cpVoiceToken, NULL);

        // Get attributes of the voice
        const QMap<QString, QString> vAttr = voiceAttributes(cpVoiceToken);

        // Transform Windows LCID to QLocale
        const QLocale vLocale = lcidToLocale(vAttr[u"Language"_s]);

        // Create voice
        const QString name = vAttr[u"Name"_s];
        const QVoice::Age age = toVoiceAge(vAttr[u"Age"_s]);
        const QVoice::Gender gender = vAttr[u"Gender"_s] == u"Male"_s ? QVoice::Male :
                                      vAttr[u"Gender"_s] == u"Female"_s ? QVoice::Female :
                                      QVoice::Unknown;
        // Getting the ID of the voice to set the voice later
        const QVoice voice = createVoice(name, vLocale, gender, age, voiceId(cpVoiceToken));
        m_voices.insert(vLocale, voice);
    }
    if (cpVoiceToken)
        cpVoiceToken->Release();
    cpEnum->Release();
}

QList<QLocale> QTextToSpeechEngineSapi::availableLocales() const
{
    return m_voices.uniqueKeys();
}

bool QTextToSpeechEngineSapi::setLocale(const QLocale &locale)
{
    const QList<QVoice> voicesForLocale = m_voices.values(locale);
    if (voicesForLocale.isEmpty()) {
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "No voice found for locale %1.")
                    .arg(locale.bcp47Name()));
        return false;
    }

    setVoice(voicesForLocale[0]);
    return true;
}

QLocale QTextToSpeechEngineSapi::locale() const
{
    // Get current voice id
    ISpObjectToken *cpVoiceToken = nullptr;
    HRESULT hr = m_voice->GetVoice(&cpVoiceToken);
    if (FAILED(hr)) {
        qWarning() << "ISpObjectToken::GetVoice failed";
        return QLocale();
    }
    // read attributes
    const QMap<QString, QString> vAttr = voiceAttributes(cpVoiceToken);
    cpVoiceToken->Release();
    return lcidToLocale(vAttr.value(u"Language"_s));
}

QList<QVoice> QTextToSpeechEngineSapi::availableVoices() const
{
    return m_voices.values(locale());
}

bool QTextToSpeechEngineSapi::setVoice(const QVoice &voice)
{
    // Convert voice id to null-terminated wide char string
    const QString vId = voiceData(voice).toString();
    QVarLengthArray<wchar_t, 255> tokenId(vId.size() + 1);
    vId.toWCharArray(tokenId.data());
    tokenId[vId.size()] = 0;

    // create the voice token via the id
    HRESULT hr = S_OK;
    ISpObjectToken *cpVoiceToken = nullptr;
    hr = SpCreateNewToken(tokenId.constData(), &cpVoiceToken);
    if (FAILED(hr)) {
        if (cpVoiceToken)
            cpVoiceToken->Release();
        setError(QTextToSpeech::ErrorReason::Configuration,
                 QCoreApplication::translate("QTextToSpeech", "Could not set voice."));
        return false;
    }

    if (m_state != QTextToSpeech::Ready) {
        m_state = QTextToSpeech::Ready;
        emit stateChanged(m_state);
    }

    m_voice->SetVoice(cpVoiceToken);
    cpVoiceToken->Release();
    return true;
}

QVoice QTextToSpeechEngineSapi::voice() const
{
    ISpObjectToken *cpVoiceToken = nullptr;
    HRESULT hr = m_voice->GetVoice(&cpVoiceToken);
    if (FAILED(hr)) {
        qWarning() << "ISpObjectToken::GetVoice failed";
        return QVoice();
    }
    QString vId = voiceId(cpVoiceToken);
    cpVoiceToken->Release();
    for (const QVoice &voice : m_voices) {
        if (voiceData(voice).toString() == vId)
            return voice;
    }
    return QVoice();
}

QTextToSpeech::State QTextToSpeechEngineSapi::state() const
{
    return m_state;
}

void QTextToSpeechEngineSapi::setError(QTextToSpeech::ErrorReason reason, const QString &string)
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

QTextToSpeech::ErrorReason QTextToSpeechEngineSapi::errorReason() const
{
    return m_errorReason;
}

QString QTextToSpeechEngineSapi::errorString() const
{
    return m_errorString;
}

HRESULT QTextToSpeechEngineSapi::NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    const QTextToSpeech::State oldState = m_state;

    // we can't use CSpEvent here as it doesn't provide the time offset
    if (ISpEventSource2 *eventSource; S_OK == m_voice->QueryInterface(&eventSource)) {
        SPEVENTEX event;
        ULONG got;
        while (S_OK == eventSource->GetEventsEx(1, &event, &got)) {
            switch (event.eEventId) {
            case SPEI_START_INPUT_STREAM:
                m_state = QTextToSpeech::Speaking;
                break;
            case SPEI_END_INPUT_STREAM:
                m_state = QTextToSpeech::Ready;
                break;
            case SPEI_WORD_BOUNDARY:
                if (qsizetype(event.lParam + event.wParam) <= currentText.size()) {
                    emit sayingWord(currentText.sliced(event.lParam, event.wParam),
                                    event.lParam - textOffset, event.wParam);
                }
                break;
            // these are the other TTS events which might be interesting for us at some point
            case SPEI_SENTENCE_BOUNDARY:
            case SPEI_PHONEME:
            case SPEI_TTS_BOOKMARK:
            case SPEI_VISEME:
            case SPEI_VOICE_CHANGE:
            case SPEI_TTS_AUDIO_LEVEL:
                break;
            // the rest is uninteresting (speech recognition or engine-internal events).
            default:
                break;
            }
            // There is no SpClearEventEx, and while a SPEVENTEX is not a subclass of SPEVENT, the
            // two structs have identical memory layout, plus the extra ullAudioTimeOffset member.
            SpClearEvent(reinterpret_cast<SPEVENT*>(&event));
        }

        eventSource->Release();
    }

    // There are no explicit events for pause/resume, so we have to brute force this ourselves.
    // This means that we transition into pause prematurely, as SAPI typically only pauses in
    // a word- or sentence-break.
    if (m_pauseRequested)
        m_state = QTextToSpeech::Paused;
    else if (m_state == QTextToSpeech::Paused && isSpeaking())
        m_state = QTextToSpeech::Speaking;

    if (m_state != oldState)
        emit stateChanged(m_state);

    return S_OK;
}

QT_END_NAMESPACE
