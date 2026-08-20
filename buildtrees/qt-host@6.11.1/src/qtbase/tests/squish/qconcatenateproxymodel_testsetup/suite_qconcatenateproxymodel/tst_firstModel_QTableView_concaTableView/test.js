// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qconcatenatetablesproxymodel");
    sendEvent("QResizeEvent", waitForObject(names.firstModelInQTreeViewQTreeView), 393, 192, 646, 338);
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "0/0"), 77, 22, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit), "x");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "1/0"), 73, 10, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_2), "y");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "2/0"), 71, 12, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_3), "z");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "3/0"), 70, 8, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_4), "zz");
    test.compare(waitForObjectExists(names.firstModelInQTableView00QModelIndex).displayText, "x");
    test.compare(waitForObjectExists(names.firstModelInQTableView10QModelIndex).displayText, "y");
    test.compare(waitForObjectExists(names.firstModelInQTableView20QModelIndex).displayText, "z");
    test.compare(waitForObjectExists(names.firstModelInQTableView30QModelIndex).displayText, "First 3,0");

    test.compare(waitForObjectExists(names.concatProxyInQTableView00QModelIndex).displayText, "x");
    test.compare(waitForObjectExists(names.concatProxyInQTableView10QModelIndex).displayText, "y");
    test.compare(waitForObjectExists(names.concatProxyInQTableView20QModelIndex).displayText, "z");
}
