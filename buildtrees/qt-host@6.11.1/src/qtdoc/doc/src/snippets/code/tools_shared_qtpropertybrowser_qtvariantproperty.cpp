// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QtVariantPropertyManager *variantPropertyManager;
        QtProperty *property;

        variantPropertyManager->setValue(property, 10);
//! [0]


//! [1]
        QtVariantPropertyManager *variantPropertyManager;
        QtVariantProperty *property;

        property->setValue(10);
//! [1]


