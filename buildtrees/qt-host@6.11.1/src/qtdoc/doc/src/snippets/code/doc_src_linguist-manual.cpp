// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [3]
label->setText(tr("F\374r \310lise"));
//! [3]


void wrapInFunction()
{
//! [6]
button = new QPushButton("&Quit", this);
//! [6]


//! [7]
button = new QPushButton(tr("&Quit"), this);
//! [7]


//! [8]
QPushButton::tr("&Quit")
//! [8]


//! [9]
QObject::tr("&Quit")
//! [9]


//! [10]
rbc = new QRadioButton(tr("Enabled", "Color frame"), this);
//! [10]


//! [11]
rbh = new QRadioButton(tr("Enabled", "Hue frame"), this);
//! [11]
}


//! [12]
/*
    TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the
    Find dialog.

    ...
*/
//! [12]

//! [13]
/*
    TRANSLATOR MyNamespace::MyClass

    Necessary for lupdate.

    ...
*/
//! [13]

//! [14]
void some_global_function(LoginWidget *logwid)
{
    QLabel *label = new QLabel(
            LoginWidget::tr("Password:"), logwid);
}

void same_global_function(LoginWidget *logwid)
{
    QLabel *label = new QLabel(
            QCoreApplication::translate("LoginWidget", "Password:"),
            logwid);
}
//! [14]


//! [15]
QString FriendlyConversation::greeting(int greet_type)
{
    static const char* greeting_strings[] = {
        QT_TR_NOOP("Hello"),
        QT_TR_NOOP("Goodbye")
    };
    return tr(greeting_strings[greet_type]);
}
//! [15]


//! [16]
static const char* greeting_strings[] = {
    QT_TRANSLATE_NOOP("FriendlyConversation", "Hello"),
    QT_TRANSLATE_NOOP("FriendlyConversation", "Goodbye")
};

QString FriendlyConversation::greeting(int greet_type)
{
    return tr(greeting_strings[greet_type]);
}

QString global_greeting(int greet_type)
{
    return QCoreApplication::translate("FriendlyConversation",
                                       greeting_strings[greet_type]);
}
//! [16]

void wrapInFunction()
{

//! [17]
QString tr(const char *text, const char *comment, int n);
//! [17]

//! [18]
tr("%n item(s) replaced", "", count);
//! [18]

}
