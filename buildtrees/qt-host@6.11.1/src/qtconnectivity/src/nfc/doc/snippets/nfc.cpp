// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtNfc/qndefrecord.h>
#include <QtNfc/qndefnfctextrecord.h>

#include <QtCore/QDebug>

QT_USE_NAMESPACE

void snippet_recordConversion()
{
    QNdefRecord record;

    //! [Record conversion]
    if (record.isRecordType<QNdefNfcTextRecord>()) {
        QNdefNfcTextRecord textRecord(record);

        qDebug() << textRecord.text();
    }
    //! [Record conversion]
}

//! [Specialized class definition]
class ExampleComF : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(ExampleComF, QNdefRecord::ExternalRtd, "example.com:f",
                          QByteArray(sizeof(int), char(0)))

    int foo() const;
    void setFoo(int v);
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(ExampleComF, QNdefRecord::ExternalRtd, "example.com:f")
//! [Specialized class definition]
