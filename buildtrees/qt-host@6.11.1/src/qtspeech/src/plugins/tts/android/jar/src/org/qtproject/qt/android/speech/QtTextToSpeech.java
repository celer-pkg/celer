// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

package org.qtproject.qt.android.speech;

import android.content.ContentResolver;
import android.content.Context;
import android.media.AudioFormat;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.speech.tts.UtteranceProgressListener;
import android.os.Bundle;
import android.util.Log;
import java.lang.Float;
import java.io.File;
import java.util.Locale;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;

class QtTextToSpeech
{
    private static final String UTTERANCE_ID = "UtteranceId";
    private static final String SYNTHESIZE_ID = "SynthesizeId";

    // Native callback functions
    native void notifyError(long id, long reason);
    native void notifyReady(long id);
    native void notifySpeaking(long id);
    native void notifyRangeStart(long id, int start, int end, int frame);
    native void notifyBeginSynthesis(long id, int sampleRateInHz, int audioFormat, int channelCount);
    native void notifyAudioAvailable(long id, byte[] bytes);
    native void notifyEndSynthesis(long id);

    private TextToSpeech mTts;
    private final long mId;
    private boolean mInitialized = false;
    private float mPitch = 1.0f;
    private float mRate = 1.0f;
    private float mVolume = 1.0f;

    private static final String TAG = "QtTextToSpeech";
    // OnInitListener
    private final OnInitListener mTtsChangeListener = new OnInitListener() {
        @Override
        public void onInit(int status) {
            if (status == TextToSpeech.SUCCESS) {
                mInitialized = true;
                notifyReady(mId);
                Log.d(TAG, "TTS initialized");
            } else {
                mInitialized = false;
                notifyError(mId, 1); // QTextToSpeech::ErrorReason::Initialization
                Log.w(TAG, "TTS initialization failed");
            }
        }
    };

    private String utteranceTAG = "UtteranceProgressListener";
    // UtteranceProgressListener
    private final UtteranceProgressListener mTtsUtteranceProgressListener = new UtteranceProgressListener() {
        @Override
        public void onDone(String utteranceId) {
            Log.d(utteranceTAG, "onDone");
            if (utteranceId.equals(UTTERANCE_ID)) {
                notifyReady(mId);
            } else if (utteranceId.equals(SYNTHESIZE_ID)) {
                notifyEndSynthesis(mId);
            }
        }

        @Override
        public void onError(String utteranceId) {
            onError(utteranceId, TextToSpeech.ERROR);
        }

        @Override
        public void onError(String utteranceId, int errorCode) {
            Log.w(utteranceTAG, "onError");
            if (utteranceId.equals(UTTERANCE_ID)) {
                switch (errorCode) {
                default:
                    notifyError(mId, 4);  // QTextToSpeech::ErrorReason::Playback
                }
            }
        }

        @Override
        public void onStart(String utteranceId) {
            Log.d(utteranceTAG, "onStart");
            if (utteranceId.equals(UTTERANCE_ID)) {
                notifySpeaking(mId);
            }
        }

        @Override
        public void onRangeStart(String utteranceId, int start, int end, int frame) {
            Log.w("UtteranceProgressListener", "onRangeStart");
            if (utteranceId.equals("UtteranceId")) {
                notifyRangeStart(mId, start, end, frame);
            }
        }

        @Override
        public void onBeginSynthesis(String utteranceId, int sampleRateInHz, int audioFormat, int channelCount) {
            Log.d(utteranceTAG, "onBeginSynthesis");
            if (utteranceId.equals(SYNTHESIZE_ID)) {
                switch (audioFormat) {
                case AudioFormat.ENCODING_PCM_8BIT:
                    audioFormat = 1; // QAudioFormat::UInt8
                    break;
                case AudioFormat.ENCODING_PCM_16BIT:
                    audioFormat = 2; // QAudioFormat::Int16;
                    break;
                case AudioFormat.ENCODING_PCM_FLOAT:
                    audioFormat = 4; // QAudioFormat::Float;
                    break;
                default:
                    audioFormat = 0; // QAudioFormat::Unknown;
                }

                notifyBeginSynthesis(mId, sampleRateInHz, audioFormat, channelCount);
            }
        }

        @Override
        public void onAudioAvailable(String utteranceId, byte[] bytes) {
            Log.d(utteranceTAG, "onAudioAvailable");
            if (utteranceId.equals(SYNTHESIZE_ID)) {
                notifyAudioAvailable(mId, bytes);
            }
        }
    };

    QtTextToSpeech(final Context context, final long id, String engine) {
        mId = id;
        if (engine.isEmpty()) {
            mTts = new TextToSpeech(context, mTtsChangeListener);
        } else {
            mTts = new TextToSpeech(context, mTtsChangeListener, engine);
        }
        mTts.setOnUtteranceProgressListener(mTtsUtteranceProgressListener);

        // Read pitch from settings
        ContentResolver resolver = context.getContentResolver();
        try {
            float pitch = Settings.Secure.getFloat(resolver, android.provider.Settings.Secure.TTS_DEFAULT_PITCH);
            mPitch = pitch / 100.0f;
        } catch (SettingNotFoundException e) {
            mPitch = 1.0f;
        }

        // Read rate from settings
        try {
            float rate = Settings.Secure.getFloat(resolver, android.provider.Settings.Secure.TTS_DEFAULT_RATE);
            mRate = rate / 100.0f;
        } catch (SettingNotFoundException e) {
            mRate = 1.0f;
        }
    }

    void say(String text)
    {
        Log.d(TAG, "TTS say(): " + text);
        int result = -1;

        Bundle params = new Bundle();
        params.putFloat(TextToSpeech.Engine.KEY_PARAM_VOLUME, mVolume);
        result = mTts.speak(text, TextToSpeech.QUEUE_FLUSH, params, UTTERANCE_ID);

        Log.d(TAG, "TTS say() result: " + Integer.toString(result));
        if (result == TextToSpeech.ERROR)
            notifyError(mId, 3); // QTextToSpeech::ErrorReason::Input
    }

    int synthesize(String text)
    {
        Log.d(TAG, "TTS synthesize(): " + text);
        int result = -1;

        Bundle params = new Bundle();
        params.putFloat(TextToSpeech.Engine.KEY_PARAM_VOLUME, mVolume);
        File file = new File("/dev/null");
        result = mTts.synthesizeToFile(text, params, file, SYNTHESIZE_ID);

        Log.d(TAG, "TTS synthesize() result: " + Integer.toString(result));
        if (result == TextToSpeech.ERROR)
            notifyError(mId, 3); // QTextToSpeech::ErrorReason::Input
        return -1;
    }

    void stop()
    {
        Log.d(TAG, "Stopping TTS");
        mTts.stop();
    }

    float pitch()
    {
        return mPitch;
    }

    int setPitch(float pitch)
    {
        if (Float.compare(pitch, mPitch) == 0)
            return TextToSpeech.ERROR;

        int success = mTts.setPitch(pitch);
        if (success == TextToSpeech.SUCCESS)
            mPitch = pitch;
        else
            notifyError(mId, 2); // QTextToSpeech::ErrorReason::Configuration

        return success;
    }

    float rate()
    {
        return mRate;
    }

    int setRate(float rate)
    {
        if (Float.compare(rate, mRate) == 0)
            return TextToSpeech.ERROR;

        int success = mTts.setSpeechRate(rate);
        if (success == TextToSpeech.SUCCESS)
            mRate = rate;
        else
            notifyError(mId, 2); // QTextToSpeech::ErrorReason::Configuration

        return success;
    }

    void shutdown()
    {
        mTts.shutdown();
    }

    float volume()
    {
        return mVolume;
    }

    int setVolume(float volume)
    {
        if (Float.compare(volume, mVolume) == 0)
            return TextToSpeech.ERROR;

        mVolume = volume;
        return TextToSpeech.SUCCESS;
    }

    boolean setLocale(Locale locale)
    {
        int result = mTts.setLanguage(locale);
        return (result != TextToSpeech.LANG_NOT_SUPPORTED) && (result != TextToSpeech.LANG_MISSING_DATA);
    }

    List<Object> getAvailableVoices()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            //Log.d(TAG, "Voices: " + mTts.getVoices());
            return new ArrayList<Object>(mTts.getVoices());
        }
        return new ArrayList<Object>();
    }

    List<Locale> getAvailableLocales()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            //Log.d(TAG, "Locales: " + mTts.getAvailableLanguages());
            final Set<Locale> languages = mTts.getAvailableLanguages();
            ArrayList<Locale> locales = new ArrayList<Locale>();

            for (Locale language : languages) {
                String languageCode = language.getLanguage();
                String countryCode = language.getCountry();

                if (languageCode.equals(language.getISO3Language()))
                    languageCode = convertLanguageCodeThreeDigitToTwoDigit(languageCode);
                if (countryCode.equals(language.getISO3Country()))
                    countryCode = convertCountryCodeThreeDigitToTwoDigit(countryCode);

                locales.add(new Locale(languageCode, countryCode));
            }

            return locales;
        }
        return new ArrayList<Locale>();
    }

    Locale getLocale()
    {
        //Log.d(TAG, "getLocale: " + mLocale);
        final Locale language = mTts.getLanguage();
        if (language == null)
            return null;
        String languageCode = language.getLanguage();
        String countryCode = language.getCountry();

        if (languageCode.equals(language.getISO3Language()))
            languageCode = convertLanguageCodeThreeDigitToTwoDigit(languageCode);
        if (countryCode.equals(language.getISO3Country()))
            countryCode = convertCountryCodeThreeDigitToTwoDigit(countryCode);

        return new Locale(languageCode, countryCode);
    }

    Object getVoice()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            return mTts.getVoice();
        }
        return null;
    }

    boolean setVoice(String voiceName)
    {
        if (!mInitialized)
            return false;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
             for (android.speech.tts.Voice voice : mTts.getVoices()) {
                 if (voice.getName().equals(voiceName)) {
                     int result = mTts.setVoice(voice);
                     if (result == TextToSpeech.SUCCESS) {
                         //Log.d(TAG, "setVoice: " + voice);
                         return true;
                     }
                     break;
                 }
             }
        }
        return false;
    }

    private String convertLanguageCodeThreeDigitToTwoDigit(String iso3Language)
    {
        final String[] isoLanguages = Locale.getISOLanguages();

        for (String isoLanguage : isoLanguages) {
            if (iso3Language.equals(new Locale(isoLanguage).getISO3Language())) {
                return isoLanguage;
            }
        }

        return iso3Language;
    }

    private String convertCountryCodeThreeDigitToTwoDigit(String iso3Country)
    {
        final String[] isoCountries = Locale.getISOCountries();

        for (String isoCountry : isoCountries) {
            if (iso3Country.equals(new Locale("en", isoCountry).getISO3Country())) {
                return isoCountry;
            }
        }

        return iso3Country;
    }
}
