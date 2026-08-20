// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef CHECKBOXWRAPPER_H
#define CHECKBOXWRAPPER_H

#include <QObject>

class QCheckBox;

class CheckBoxWrapper : public QObject
{
    Q_OBJECT
public:
    explicit CheckBoxWrapper(QCheckBox *cb);

public Q_SLOTS:
    void setEnabled(int enabled);

private:
    QCheckBox *m_checkbox;
};

#endif // CHECKBOXWRAPPER_H
