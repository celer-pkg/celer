// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQml.Models


//! [CategoryModel view]
ListView {
    id: root
    property var categoryModel
    property var rootIndex

    signal searchCategory(var category)
    signal showSubcategories(var index)

    snapMode: ListView.SnapToItem

    model: DelegateModel {
        id: delegeteDataModel
        model: root.categoryModel
        rootIndex: root.rootIndex
        delegate: CategoryDelegate {
            width: ListView.view.width
            onSearchCategory: root.searchCategory(category);
            onShowSubcategories: root.showSubcategories(delegeteDataModel.modelIndex(index));
        }
    }
}
//! [CategoryModel view]
