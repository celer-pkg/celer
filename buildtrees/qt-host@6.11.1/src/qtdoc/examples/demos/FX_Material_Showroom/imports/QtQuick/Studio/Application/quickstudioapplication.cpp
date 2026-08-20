// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "quickstudioapplication_p.h"

#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(texttomodelMergerDebug, "qt.Studioapplication.debug", QtDebugMsg);

QuickStudioApplication::QuickStudioApplication(QObject *parent) : QObject(parent) { }

static void loadFont(const QString &path)
{
    qCDebug(texttomodelMergerDebug) << Q_FUNC_INFO << "Load font: " << path;
    QFontDatabase::addApplicationFont(path);
}

void QuickStudioApplication::setFontPath(const QUrl &url)
{
    if (url == fontPath())
        return;

    m_fontPath = url;

    QString localPath;

    if (url.isLocalFile())
        localPath = url.toLocalFile();

    if (url.scheme() == QStringLiteral("qrc")) {
        const QString &path = url.path();
        localPath = QStringLiteral(":") + path;
    }

    if (!localPath.isEmpty()) {
        QDirIterator it(localPath, { QStringLiteral("*.ttf"), QStringLiteral("*.otf") },
                        QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext())
            loadFont(it.next());
    }

    emit fontPathChanged();
}
