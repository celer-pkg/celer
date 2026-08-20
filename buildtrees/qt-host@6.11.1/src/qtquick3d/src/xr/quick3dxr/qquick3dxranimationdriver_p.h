// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRANIMATIONDRIVER_P_H
#define QQUICK3DXRANIMATIONDRIVER_P_H

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


#include <QtCore/QAnimationDriver>

QT_BEGIN_NAMESPACE

class QQuick3DXrAnimationDriver : public QAnimationDriver
{
public:
    QQuick3DXrAnimationDriver();
    void advance() override;
    qint64 elapsed() const override;

    void setStep(int stepSize);

protected:
    void start() override;

private:
    int m_step = 16;
    qint64 m_elapsed = 0;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRANIMATIONDRIVER_P_H
