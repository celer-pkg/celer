// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#undef QT_NO_FOREACH // this file contains unported legacy Q_FOREACH uses

#include <QMimeData>
#include <QDragEnterEvent>
#include <QImage>

namespace MyNamespace
{
    class MyWidget
    {
        public:
            void dragEnterEvent(QDragEnterEvent *event);
            void dropEvent(QDropEvent *event);
    };

    //! [0]
    void MyWidget::dragEnterEvent(QDragEnterEvent *event)
    {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
    }

    void MyWidget::dropEvent(QDropEvent *event)
    {
        if (event->mimeData()->hasUrls()) {
            foreach (QUrl url, event->mimeData()->urls()) {
                //...
            }
        }
    }
    //! [0]
}

void wrap()
{
    QByteArray something = "";
    //! [1]
    QByteArray csvData = something;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("text/csv", csvData);
    //! [1]
}

namespace OtherNamespace
{

    class MyWidget
    {
        public:
            void dropEvent(QDropEvent *event);
    };

    class MyMimeData : public QMimeData
    {
        Q_OBJECT
    };

    //! [2]
    void MyWidget::dropEvent(QDropEvent *event)
    {
        const MyMimeData *myData =
                qobject_cast<const MyMimeData *>(event->mimeData());
        if (myData) {
            // access myData's data directly (not through QMimeData's API)
        }
    }
    //! [2]
}

#if PRO_FILE
//! [3]
application/x-qt-windows-mime;value="<custom type>"
//! [3]


//! [4]
application/x-qt-windows-mime;value="FileGroupDescriptor"
application/x-qt-windows-mime;value="FileContents"
//! [4]
#endif

void examples(QDropEvent *event, QMimeData *mimeData)
{
    {
        //! [5]
        if (event->mimeData()->hasImage()) {
            QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
            //...
        }
        //! [5]
    }

    {
        //! [6]
        mimeData->setImageData(QImage("beautifulfjord.png"));
        //! [6]
    }

    {
        //! [7]
        if (event->mimeData()->hasColor()) {
            QColor color = qvariant_cast<QColor>(event->mimeData()->colorData());
            //...
        }
        //! [7]
    }
}

#if PRO_FILE
//! [8]
application/x-qt-windows-mime;value="FileContents";index=0
application/x-qt-windows-mime;value="FileContents";index=1
//! [8]
#endif
