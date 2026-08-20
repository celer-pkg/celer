// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
label->setText("Password:");
//! [0]


//! [1]
label->setText(tr("Password:"));
//! [1]


//! [2]
QFile file(QString::fromLatin1("appicon.png"));
//! [2]


//! [3]
QFile file(QLatin1String("appicon.png"));
//! [3]
