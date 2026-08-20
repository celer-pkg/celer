// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQSTYLEKITCUSTOMCONTROL_P_H
#define QQSTYLEKITCUSTOMCONTROL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQml/QtQml>
#include "qqstylekitcontrol_p.h"

QT_BEGIN_NAMESPACE

class QQStyleKitCustomControl: public QQStyleKitControl
{
    Q_OBJECT
    Q_PROPERTY(int controlType READ controlType WRITE setControlType NOTIFY controlTypeChanged FINAL)
    QML_NAMED_ELEMENT(CustomControl)

public:
    QQStyleKitCustomControl(QObject *parent = nullptr);

    int controlType() const;
    void setControlType(int controlType);

signals:
    void controlTypeChanged();

private:
    Q_DISABLE_COPY(QQStyleKitCustomControl)

    int m_controlType = 0;
};

QT_END_NAMESPACE

#endif // QQSTYLEKITCUSTOMCONTROL_P_H
