// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHPLUGIN_SAPI_H
#define QTEXTTOSPEECHPLUGIN_SAPI_H

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtTextToSpeech/qtexttospeechplugin.h>
#include <QtTextToSpeech/qtexttospeechengine.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechSapiPlugin : public QObject, public QTextToSpeechPlugin
{
    Q_OBJECT
    Q_INTERFACES(QTextToSpeechPlugin)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.speech.tts.plugin/6.0"
                      FILE "sapi_plugin.json")

public:
    QTextToSpeechEngine *createTextToSpeechEngine(
                                const QVariantMap &parameters,
                                QObject *parent,
                                QString *errorString) const override;
};

QT_END_NAMESPACE

#endif
