// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import * as names from 'names.js';

function main() {
    startApplication("qconcatenatetablesproxymodel");
    test.compare(waitForObjectExists(names.concatProxyInQTableView00QModelIndex).text, "First 0,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView01QModelIndex).text, "First 0,1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView10QModelIndex).text, "First 1,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView11QModelIndex).text, "First 1,1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView20QModelIndex).text, "First 2,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView21QModelIndex).text, "First 2,1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView30QModelIndex).text, "First 3,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView31QModelIndex).text, "First 3,1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView40QModelIndex).text, "Second 0,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView41QModelIndex).text, "Second 0,1");
    test.compare(waitForObjectExists(names.concatProxyInQTableView50QModelIndex).text, "Second 1,0");
    test.compare(waitForObjectExists(names.concatProxyInQTableView51QModelIndex).text, "Second 1,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst00QModelIndex).text, "First 0,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst01QModelIndex).text, "First 0,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst10QModelIndex).text, "First 1,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst11QModelIndex).text, "First 1,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst20QModelIndex).text, "First 2,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst21QModelIndex).text, "First 2,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst30QModelIndex).text, "First 3,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewFirst31QModelIndex).text, "First 3,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewSecond00QModelIndex).text, "Second 0,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewSecond01QModelIndex).text, "Second 0,1");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewSecond10QModelIndex).text, "Second 1,0");
    test.compare(waitForObjectExists(names.concatProxyInQTreeViewSecond11QModelIndex).text, "Second 1,1");
    test.compare(waitForObjectExists(names.firstModelInQTableView00QModelIndex).text, "First 0,0");
    test.compare(waitForObjectExists(names.firstModelInQTableView01QModelIndex).text, "First 0,1");
    test.compare(waitForObjectExists(names.firstModelInQTableView02QModelIndex).text, "First 0,2");
    test.compare(waitForObjectExists(names.firstModelInQTableView03QModelIndex).text, "First 0,3");
    test.compare(waitForObjectExists(names.firstModelInQTableView10QModelIndex).text, "First 1,0");
    test.compare(waitForObjectExists(names.firstModelInQTableView11QModelIndex).text, "First 1,1");
    test.compare(waitForObjectExists(names.firstModelInQTableView12QModelIndex).text, "First 1,2");
    test.compare(waitForObjectExists(names.firstModelInQTableView13QModelIndex).text, "First 1,3");
    test.compare(waitForObjectExists(names.firstModelInQTableView20QModelIndex).text, "First 2,0");
    test.compare(waitForObjectExists(names.firstModelInQTableView21QModelIndex).text, "First 2,1");
    test.compare(waitForObjectExists(names.firstModelInQTableView22QModelIndex).text, "First 2,2");
    test.compare(waitForObjectExists(names.firstModelInQTableView23QModelIndex).text, "First 2,3");
    test.compare(waitForObjectExists(names.firstModelInQTableView30QModelIndex).text, "First 3,0");
    test.compare(waitForObjectExists(names.firstModelInQTableView31QModelIndex).text, "First 3,1");
    test.compare(waitForObjectExists(names.firstModelInQTableView32QModelIndex).text, "First 3,2");
    test.compare(waitForObjectExists(names.firstModelInQTableView33QModelIndex).text, "First 3,3");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst00QModelIndex).text, "First 0,0");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst01QModelIndex).text, "First 0,1");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst02QModelIndex).text, "First 0,2");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst03QModelIndex).text, "First 0,3");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst10QModelIndex).text, "First 1,0");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst11QModelIndex).text, "First 1,1");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst12QModelIndex).text, "First 1,2");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst13QModelIndex).text, "First 1,3");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst20QModelIndex).text, "First 2,0");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst21QModelIndex).text, "First 2,1");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst22QModelIndex).text, "First 2,2");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst23QModelIndex).text, "First 2,3");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst30QModelIndex).text, "First 3,0");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst31QModelIndex).text, "First 3,1");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst32QModelIndex).text, "First 3,2");
    test.compare(waitForObjectExists(names.firstModelInQTreeViewFirst33QModelIndex).text, "First 3,3");
}
