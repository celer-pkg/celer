// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QWidget>
#include <QAbstractButton>
#include <QPushButton>

//! [4]
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "4.0.2")

    QApplication app(argc, argv);
    //...
    return app.exec();
}
//! [4]

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    void QMessageBoxExample(QWidget &myWidget);
};

void MyWidget::QMessageBoxExample(QWidget &myWidget)
{
    {
        //! [0]
        int ret = QMessageBox::warning(this, tr("My Application"),
                                       tr("The document has been modified.\n"
                                          "Do you want to save your changes?"),
                                       QMessageBox::Save | QMessageBox::Discard
                                       | QMessageBox::Cancel,
                                       QMessageBox::Save);
        //! [0]
    }

    {
        //! [2]
        QMessageBox msgBox(this);
        QPushButton *connectButton = msgBox.addButton(tr("Connect"), QMessageBox::ActionRole);
        QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);

        msgBox.exec();

        if (msgBox.clickedButton() == connectButton) {
            // connect
        } else if (msgBox.clickedButton() == abortButton) {
            // abort
        }
        //! [2]
    }

    {
        //! [3]
        QMessageBox messageBox(this);
        QAbstractButton *disconnectButton =
              messageBox.addButton(tr("Disconnect"), QMessageBox::ActionRole);
        //...
        messageBox.exec();
        if (messageBox.clickedButton() == disconnectButton) {
            //...
        }
        //! [3]
    }

    {
        //! [5]
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.exec();
        //! [5]
    }

    {
        //! [6]
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        //! [6]
    }

    {
        int ret;

        //! [7]
        switch (ret) {
            case QMessageBox::Save:
                // Save was clicked
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                break;
            default:
                // should never be reached
                break;
        }
        //! [7]
    }
}
