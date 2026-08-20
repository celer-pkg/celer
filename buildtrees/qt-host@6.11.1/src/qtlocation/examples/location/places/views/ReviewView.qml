// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation

//! [ReviewModel delegate]
ListView {
    id:view
    property Place place
    signal showReview(var review)
    model: place.reviewModel
    delegate: ReviewDelegate {
        width: ListView.view.width
        onShowReview: view.showReview(model)
    }
}
//! [ReviewModel delegate]

