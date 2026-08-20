// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_android_plugin.h"
#include "qtexttospeech_android.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSpeechTtsAndroid, "qt.speech.tts.android")

QTextToSpeechEngine *QTextToSpeechPluginAndroid::createTextToSpeechEngine(
        const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    QTextToSpeechEngineAndroid *android = new QTextToSpeechEngineAndroid(parameters, parent);
    if (android) {
        return android;
    }
    delete android;
    return 0;
}

QT_END_NAMESPACE
