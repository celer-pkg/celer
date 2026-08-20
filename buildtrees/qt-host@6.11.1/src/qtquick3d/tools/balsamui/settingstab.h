// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QString>
#include <QScrollArea>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

struct Setting
{
    QWidget *uiELement = nullptr;
    QString name;
    bool defaultBool = false;
    double defaultReal = 0.0;
};

class SettingsTab : public QScrollArea
{
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);

    QJsonObject getOptions() const;

private:
    QList<Setting> settings;
};

#endif
