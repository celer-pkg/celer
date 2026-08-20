// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_classwithattributetest_merged.h"

#include <QTest>

class tst_RepcInLib : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void instantiateClassesFromLib();
};

void tst_RepcInLib::instantiateClassesFromLib()
{
    MyPod mypod;
    MyAttributeClassSimpleSource source;
    MyAttributeClassSourceAPI<MyAttributeClassSimpleSource> sourceAPI(&source);
    MyAttributeClassReplica replica;
}

QTEST_APPLESS_MAIN(tst_RepcInLib)

#include "tst_repcinlib.moc"
