// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QProcess *process = new QProcess(this);
        QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath)
            + QLatin1String("/assistant");

        process->start(app, QStringList() << QLatin1String("-enableRemoteControl"));
        if (!process->waitForStarted()) {
            QMessageBox::critical(this, tr("Remote Control"),
                tr("Could not start Qt Assistant from %1.").arg(app));
            return;
        }

        // show index page
        QTextStream str(process);
        str << QLatin1String("SetSource qthelp://mycompany.com/doc/index.html")
            << QLatin1Char('\0') << endl;
    }
//! [0]


//! [1]
        CONFIG += assistant
//! [1]


