// Copyright (C) 2025 Jarek Kobus
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PROGRESSINDICATOR_H
#define PROGRESSINDICATOR_H

#include <QObject>

class ProgressIndicator : public QObject
{
public:
    ProgressIndicator(QWidget *parent = nullptr);

    void show();
    void hide();

private:
    class ProgressIndicatorWidget *m_widget = nullptr;
};

#endif // PROGRESSINDICATOR_H
