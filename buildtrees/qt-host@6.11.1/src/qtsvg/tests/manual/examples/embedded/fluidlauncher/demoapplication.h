// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEMO_APPLICATION_H
#define DEMO_APPLICATION_H

#include <QImage>
#include <QProcess>
#include <QObject>

class DemoApplication : public QObject
{
  Q_OBJECT

public:
    DemoApplication(QString executableName, QString caption, QString imageName, QStringList args);
    void launch();
    QImage getImage() const;
    QString getCaption();

public slots:
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError err);

signals:
    void demoStarted();
    void demoFinished();

private:
    QString imagePath;
    QString appCaption;
    QString executablePath;
    QStringList arguments;
    QProcess process;
};




#endif


