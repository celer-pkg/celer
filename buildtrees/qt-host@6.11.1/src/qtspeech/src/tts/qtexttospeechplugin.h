// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHPLUGIN_H
#define QTEXTTOSPEECHPLUGIN_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the Qt TextToSpeech plugin API, with limited compatibility
// guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtTextToSpeech/qtexttospeechengine.h>

#include <QtCore/QtPlugin>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE

class Q_TEXTTOSPEECH_EXPORT QTextToSpeechPlugin
{
public:
    virtual ~QTextToSpeechPlugin() {}

    virtual QTextToSpeechEngine *createTextToSpeechEngine(
            const QVariantMap &parameters,
            QObject *parent,
            QString *errorString) const;
};

Q_DECLARE_INTERFACE(QTextToSpeechPlugin,
                    "org.qt-project.qt.speech.tts.plugin/6.0")

QT_END_NAMESPACE

#endif
