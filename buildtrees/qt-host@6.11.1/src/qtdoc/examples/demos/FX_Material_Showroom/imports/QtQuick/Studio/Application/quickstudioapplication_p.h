// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef QUICKSTUDIOAPPLICATION_P_H
#define QUICKSTUDIOAPPLICATION_P_H

#include <QtCore/qurl.h>
#include <QtQml/qqml.h>

class QuickStudioApplication : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(StudioApplication)

    Q_PROPERTY(QUrl fontPath READ fontPath WRITE setFontPath NOTIFY fontPathChanged)

public:
    explicit QuickStudioApplication(QObject *parent = nullptr);

    const QUrl fontPath() { return m_fontPath; }
    void setFontPath(const QUrl &path);

signals:
    void fontPathChanged();

private:
    QUrl m_fontPath;
};

QML_DECLARE_TYPE(QuickStudioApplication)

#endif // QUICKSTUDIOAPPLICATION_P_H
