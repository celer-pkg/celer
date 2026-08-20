// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import { Backend } from "MyApp"

//! [file]
export function backendStatusUpdate(backendStatus) {
    if (backendStatus === Backend.Error) {
        console.warn("Error!")
        return
    }

    console.log("Backend loaded successfully")
}
//! [file]
