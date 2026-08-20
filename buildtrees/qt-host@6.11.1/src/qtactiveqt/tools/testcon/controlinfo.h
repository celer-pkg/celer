// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CONTROLINFO_H
#define CONTROLINFO_H

#include <QtCore/qglobal.h>
#include "ui_controlinfo.h"

QT_BEGIN_NAMESPACE

class ControlInfo : public QDialog, Ui::ControlInfo
{
    Q_OBJECT
public:
    ControlInfo(QWidget *parent);

    void setControl(QWidget *activex);
};

QT_END_NAMESPACE

#endif // CONTROLINFO_H
