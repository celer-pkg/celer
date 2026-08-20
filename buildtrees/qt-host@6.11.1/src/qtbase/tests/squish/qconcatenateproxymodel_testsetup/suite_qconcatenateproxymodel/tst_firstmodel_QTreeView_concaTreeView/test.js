// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qconcatenatetablesproxymodel");
    doubleClick(waitForObjectItem(names.firstModelInQTreeViewQTreeView, "First 0,0"), 42, 5, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTreeViewQExpandingLineEdit), "1");
    doubleClick(waitForObjectItem(names.firstModelInQTreeViewQTreeView, "First 1,0"), 43, 10, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTreeViewQExpandingLineEdit_2), "2");
    doubleClick(waitForObjectItem(names.firstModelInQTreeViewQTreeView, "First 2,0"), 46, 3, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTreeViewQExpandingLineEdit_3), "3");
    mouseClick(waitForObject(names.firstModelInQTreeViewQExpandingLineEdit_3), 14, 17, Qt.NoModifier, Qt.LeftButton);
    doubleClick(waitForObjectItem(names.firstModelInQTreeViewQTreeView, "First 3,0"), 31, 9, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTreeViewQExpandingLineEdit_4), "4");

    test.compare(waitForObjectExists(names.concatProxyInQTableView00QModelIndex).displayText, "1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView10QModelIndex).displayText, "2");
    test.compare(waitForObjectExists(names.concatProxyInQTableView20QModelIndex).displayText, "3");
    test.compare(waitForObjectExists(names.concatProxyInQTableView30QModelIndex).displayText, "First 3,0");
}
