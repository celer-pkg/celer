// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QTest>

class TestFileLocations : public QObject
{
    Q_OBJECT
private slots:
    void InfoDOMAPI();
    void NodeAPI();
    void NodeDOMAPI();
    void createEnsureFind();
    void visitTree();
    void addRegion();
    void treeOf();
};
