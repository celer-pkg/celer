// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QWidget>

namespace droparea {
struct DropArea : public QWidget {
    void paste();
    void setPixmap(QPixmap);
    void setText(QString);
    void setTextFormat(Qt::TextFormat);
};

#ifndef QT_NO_CLIPBOARD
//![0]
void DropArea::paste()
{
    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasImage()) {
        setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
    } else if (mimeData->hasHtml()) {
        setText(mimeData->html());
        setTextFormat(Qt::RichText);
    } else if (mimeData->hasText()) {
        setText(mimeData->text());
        setTextFormat(Qt::PlainText);
    } else {
        setText(tr("Cannot display data"));
    }
}
//![0]
#endif

} // droparea
