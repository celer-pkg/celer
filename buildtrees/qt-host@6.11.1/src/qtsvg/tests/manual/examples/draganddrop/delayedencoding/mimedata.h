// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MIMEDATA_H
#define MIMEDATA_H

#include <QMimeData>

//![0]
class MimeData : public QMimeData
{
    Q_OBJECT

public:
    MimeData();
    QStringList formats() const override;

signals:
    void dataRequested(const QString &mimeType) const;

protected:
    QVariant retrieveData(const QString &mimetype, QMetaType type) const override;
};
//![0]

#endif
