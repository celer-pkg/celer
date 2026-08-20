// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QMLLS_DIFFER_H
#define TST_QMLLS_DIFFER_H

#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

class tst_qmlls_differ : public QObject
{
    Q_OBJECT
public:
    tst_qmlls_differ();
private slots:
    void computeDiff_data();
    void computeDiff();
    void applyDiffs_data();
    void applyDiffs();
};

#endif // TST_QMLLS_DIFFER_H
