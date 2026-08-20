// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXOBJECTINTERFACE_H
#define QAXOBJECTINTERFACE_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QString;

class QAxObjectInterface
{
public:
    virtual ~QAxObjectInterface();

    virtual ulong classContext() const = 0;
    virtual void setClassContext(ulong classContext) = 0;

    virtual QString control() const = 0;
    virtual void resetControl() = 0;
    virtual bool setControl(const QString &c) = 0;
};

QT_END_NAMESPACE

#endif // QAXOBJECTINTERFACE_H
