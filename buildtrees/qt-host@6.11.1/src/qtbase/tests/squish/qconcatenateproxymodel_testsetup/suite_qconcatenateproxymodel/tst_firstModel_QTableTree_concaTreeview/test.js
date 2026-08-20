// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main()
{
    startApplication("qconcatenatetablesproxymodel");
    sendEvent("QResizeEvent", waitForObject(names.firstModelInQTreeViewQTreeView), 393, 192, 646, 338);
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "0/0"), 77, 22, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit), "aaa");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "1/0"), 73, 10, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_2), "bbb");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "2/0"), 71, 12, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_3), "ccc");
    doubleClick(waitForObjectItem(names.firstModelInQTableViewQTableView, "3/0"), 70, 8, Qt.NoModifier, Qt.LeftButton);
    type(waitForObject(names.firstModelInQTableViewQExpandingLineEdit_4), "ddd");
    test.compare(waitForObjectExists(names.firstModelInQTableView00QModelIndex).displayText, "aaa");
    test.compare(waitForObjectExists(names.firstModelInQTableView10QModelIndex).displayText, "bbb");
    test.compare(waitForObjectExists(names.firstModelInQTableView20QModelIndex).displayText, "ccc");
    test.compare(waitForObjectExists(names.firstModelInQTableView30QModelIndex).displayText, "First 3,0");

    test.compare(waitForObjectExists(names.concatProxyInQTableView00QModelIndex).displayText, "aaa");
    test.compare(waitForObjectExists(names.concatProxyInQTableView10QModelIndex).displayText, "bbb");
    test.compare(waitForObjectExists(names.concatProxyInQTableView20QModelIndex).displayText, "ccc");
    test.compare(waitForObjectExists(names.concatProxyInQTableView30QModelIndex).displayText, "First 3,0");
}
