// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_flite_plugin.h"
#include "qtexttospeech_flite.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSpeechTtsFlite, "qt.speech.tts.flite")

QTextToSpeechEngine *QTextToSpeechFlitePlugin::createTextToSpeechEngine(
        const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineFlite(parameters, parent);
}

QT_END_NAMESPACE
