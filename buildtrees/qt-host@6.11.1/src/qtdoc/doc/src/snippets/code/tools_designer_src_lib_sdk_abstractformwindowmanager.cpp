// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QDesignerFormWindowManagerInterface *manager = 0;
        QDesignerFormWindowInterface *formWindow = 0;

        manager = formEditor->formWindowManager();
        formWindow = manager->formWindow(0);

        manager->setActiveFormWindow(formWindow);
//! [0]


