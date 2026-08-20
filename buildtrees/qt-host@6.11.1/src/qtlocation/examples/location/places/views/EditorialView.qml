// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation

//! [PlaceEditorialModel view]
ListView {
    id:view
    property Place place
    signal showEditorial(var editorial)
    model: place.editorialModel
    delegate: EditorialDelegate {
        width: ListView.view.width
        onShowEditorial: view.showEditorial(model)
    }
}
//! [PlaceEditorialModel view]

