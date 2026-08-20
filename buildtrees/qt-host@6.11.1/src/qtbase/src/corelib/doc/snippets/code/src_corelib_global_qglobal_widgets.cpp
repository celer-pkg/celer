// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QLabel>
#include <QStyle>

void label_example()
{
    QLabel *label = new QLabel;
    //! [0]
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setAlignment({ });
    //! [0]
}

QStyle *style()
{
    static QStyle *s = nullptr;
    return s;
}

void snippet_16()
{
    //! [1]
    #if QT_VERSION >= QT_VERSION_CHECK(4, 1, 0)
        QIcon icon = style()->standardIcon(QStyle::SP_TrashIcon);
    #else
        QPixmap pixmap = style()->standardPixmap(QStyle::SP_TrashIcon);
        QIcon icon(pixmap);
    #endif
    //! [1]
}

//! [2]
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

QtMessageHandler originalHandler = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message = qFormatLogMessage(type, context, msg);
    static FILE *f = fopen("log.txt", "a");
    fprintf(f, "%s\n", qPrintable(message));
    fflush(f);

    if (originalHandler)
        originalHandler(type, context, msg);
}

int main(int argc, char **argv)
{
    originalHandler = qInstallMessageHandler(logToFile);
    QApplication app(argc, argv);
    // ...
    return app.exec();
}
//! [2]

class FriendlyConversation
{
public:
    QString greeting(int type);
};

QString tr(const char *)
{
    return "";
}


//! [3]
static const char *greeting_strings[] = {
    QT_TRANSLATE_NOOP("FriendlyConversation", "Hello"),
    QT_TRANSLATE_NOOP("FriendlyConversation", "Goodbye")
};

QString FriendlyConversation::greeting(int type)
{
    return tr(greeting_strings[type]);
}

QString global_greeting(int type)
{
    return qApp->translate("FriendlyConversation",
                           greeting_strings[type]);
}
//! [3]


namespace repetition
{
    class FriendlyConversation
    {
    public:
        QString greeting(int type);
    };

    QString tr(const char *text, const char *comment)
    {
        return "";
    }


    //! [4]

    static struct { const char *source; const char *comment; } greeting_strings[] =
    {
        QT_TRANSLATE_NOOP3("FriendlyConversation", "Hello",
                           "A really friendly hello"),
        QT_TRANSLATE_NOOP3("FriendlyConversation", "Goodbye",
                           "A really friendly goodbye")
    };

    QString FriendlyConversation::greeting(int type)
    {
        return tr(greeting_strings[type].source,
                  greeting_strings[type].comment);
    }

    QString global_greeting(int type)
    {
        return qApp->translate("FriendlyConversation",
                               greeting_strings[type].source,
                               greeting_strings[type].comment);
    }
    //! [4]
}

namespace snippet_qttranslatennoop3
{
    class FriendlyConversation
    {
    public:
        QString greeting(int type, int count);
    };

    QString tr(const char *text, const char *comment, int n)
    {
        return "";
    }

    //! [qttranslatennoop]
    static struct { const char * const source; const char * const comment; } status_strings[] = {
        QT_TRANSLATE_N_NOOP3("Message Status", "Hello, you have %n message(s)",
                             "A login message status"),
        QT_TRANSLATE_N_NOOP3("Message status", "You have %n new message(s)",
                             "A new message query status")
    };

    QString FriendlyConversation::greeting(int type, int count)
    {
        return tr(status_strings[type].source,
                  status_strings[type].comment, count);
    }

    QString global_greeting(int type, int count)
    {
        return qApp->translate("Message Status",
                               status_strings[type].source,
                               status_strings[type].comment,
                               count);
    }
    //! [qttranslatennoop]
}

class TheClass : public QWidget
{
    public:
        TheClass(QWidget *parent = nullptr) : QWidget(parent) {
            addLabels();
        }
        void addLabels();
};

//! [qttrid_noop]
static const char * const ids[] = {
    //% "This is the first text."
    QT_TRID_NOOP("qtn_1st_text"),
    //% "This is the second text."
    QT_TRID_NOOP("qtn_2nd_text"),
    0
};

void TheClass::addLabels()
{
    for (int i = 0; ids[i]; ++i)
        new QLabel(qtTrId(ids[i]), this);
}
//! [qttrid_noop]

void qwidget_example()
{
    //! [5]
    QWidget w = QWidget();
    //! [5]
}

//! [qt-version-check]
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
//! [qt-version-check]
