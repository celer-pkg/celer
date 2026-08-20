// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QProcess>
#include <QIODevice>
#include <QFile>

bool read_example()
{
    //! [0]
    QProcess gzip;
    gzip.start("gzip", QStringList() << "-c");
    if (!gzip.waitForStarted())
        return false;

    gzip.write("uncompressed data");

    QByteArray compressed;
    while (gzip.waitForReadyRead())
        compressed += gzip.readAll();
    //! [0]

    return true;
}

class CustomDevice : public QIODevice
{
    Q_OBJECT
public:
    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
private:
    QByteArray buffer;
};

//! [1]
qint64 CustomDevice::bytesAvailable() const
{
    return buffer.size() + QIODevice::bytesAvailable();
}
//! [1]


void read_in_buf_example()
{
    //! [2]
    QFile file("box.txt");
    if (file.open(QFile::ReadOnly)) {
        char buf[1024];
        qint64 lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            // the line is available in buf
        }
    }
    //! [2]
}


//! [3]
bool CustomDevice::canReadLine() const
{
    return buffer.contains('\n') || QIODevice::canReadLine();
}
//! [3]


//! [method_open]
bool isExeFile(QFile *file)
{
//! [method_open]

    if (true)
    {
        //! [method_body_0]
            char buf[2];
            if (file->peek(buf, sizeof(buf)) == sizeof(buf))
                return (buf[0] == 'M' && buf[1] == 'Z');
            return false;
        //! [method_body_0]
    }
    else
    {
        //! [method_body_1]
            return file->peek(2) == "MZ";
        //! [method_body_1]
    }

//! [method_close]
}
//! [method_close]
