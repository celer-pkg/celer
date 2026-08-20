// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <qiconengineplugin.h>
#include <qstringlist.h>

#include "qsvgiconengine.h"

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QSvgIconPlugin : public QIconEnginePlugin
{
    Q_OBJECT
#ifndef QT_NO_COMPRESS
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "qsvgiconengine.json")
#else
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "qsvgiconengine-nocompress.json")
#endif

public:
    QIconEngine *create(const QString &filename = QString()) override;
};

QIconEngine *QSvgIconPlugin::create(const QString &)
{
    QSvgIconEngine *engine = new QSvgIconEngine;
    return engine;
}

QT_END_NAMESPACE

#include "main.moc"
