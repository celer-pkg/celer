// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CONVERTERTHREAD_H
#define CONVERTERTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QDir>
#include <QStringList>
#include <QtCore/qjsonobject.h>

class ConverterThread : public QThread
{
    Q_OBJECT

public:
    ConverterThread(QObject *parent = nullptr);
    ~ConverterThread();

    void convert(QStringList filenames, QDir outputPath, QJsonObject options);

protected:
    void run() override;

signals:
    void convertStart(const QString &text);
    void convertUpdate(const QString &text);
    void convertDone(const QString &text);

private:
    QMutex mutex;
    QWaitCondition condition;

    bool abort = false;

    QStringList m_filenames;
    QDir m_outputPath;
    QJsonObject m_options;
};

#endif
