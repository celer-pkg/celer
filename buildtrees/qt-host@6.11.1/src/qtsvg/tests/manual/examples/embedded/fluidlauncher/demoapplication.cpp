// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QDebug>
#include <QDir>

#include "demoapplication.h"

DemoApplication::DemoApplication(QString executableName, QString caption, QString imageName, QStringList args)
{
    imagePath = imageName;
    appCaption = caption;

    if (executableName.startsWith(QLatin1Char('/')))
        executablePath = executableName;
    else
        executablePath = QDir::cleanPath(QDir::currentPath() + QLatin1Char('/') + executableName);
  
#ifdef WIN32
    if (!executablePath.endsWith(QLatin1String(".exe")))
        executablePath.append(QLatin1String(".exe"));
#endif

    arguments = args;

    process.setProcessChannelMode(QProcess::ForwardedChannels);

    connect(&process, &QProcess::finished, this, &DemoApplication::processFinished);
    connect(&process, &QProcess::errorOccurred, this, &DemoApplication::processError);
    connect(&process, &QProcess::started, this, &DemoApplication::processStarted);
}


void DemoApplication::launch()
{
    process.start(executablePath, arguments);
}

QImage DemoApplication::getImage() const
{
    if (imagePath.isEmpty())
        return QImage();

    // in local dir?
    QImage result(imagePath);
    if (!result.isNull())
        return result;

    // provided by qrc
    result = QImage(QString(":/fluidlauncher/%1").arg(imagePath));
    return result;
}

QString DemoApplication::getCaption()
{
    return appCaption;
}

void DemoApplication::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    emit demoFinished();

    disconnect(this, &DemoApplication::demoStarted, 0, 0);
    disconnect(this, &DemoApplication::demoFinished, 0, 0);
}

void DemoApplication::processError(QProcess::ProcessError err)
{
    qDebug() << "Process error: " << err;
    if (err == QProcess::Crashed)
        emit demoFinished();
}


void DemoApplication::processStarted()
{
    emit demoStarted();
}






