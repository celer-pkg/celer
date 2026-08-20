// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QTimer>
#include <QKeyEvent>
#include <QTextEdit>
#include <QVBoxLayout>

QString objectName() { return ""; }

void examples(int newPrecision)
{
    {
        //! [1]
        QObject *obj = new QPushButton;
        obj->metaObject()->className();             // returns "QPushButton"

        QPushButton::staticMetaObject.className();  // returns "QPushButton"
        //! [1]
    }

    {
        //! [2]
        QPushButton::staticMetaObject.className();  // returns "QPushButton"

        QObject *obj = new QPushButton;
        obj->metaObject()->className();             // returns "QPushButton"
        //! [2]
    }

    {
        //! [3]
        QObject *obj = new QTimer;          // QTimer inherits QObject

        QTimer *timer = qobject_cast<QTimer *>(obj);
        // timer == (QObject *)obj

        QAbstractButton *button = qobject_cast<QAbstractButton *>(obj);
        // button == nullptr
        //! [3]
    }

    {
        //! [4]
        QTimer *timer = new QTimer;         // QTimer inherits QObject
        timer->inherits("QTimer");          // returns true
        timer->inherits("QObject");         // returns true
        timer->inherits("QAbstractButton"); // returns false

        // QVBoxLayout inherits QObject and QLayoutItem
        QVBoxLayout *layout = new QVBoxLayout;
        layout->inherits("QObject");        // returns true
        layout->inherits("QLayoutItem");    // returns true (even though QLayoutItem is not a QObject)
        //! [4]
    }

    {
        //! [5]
        qDebug("MyClass::setPrecision(): (%s) invalid precision %f",
               qPrintable(objectName()), newPrecision);
        //! [5]
    }
}

//! [6]
class MainWindow : public QMainWindow
{
public:
    MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QTextEdit *textEdit;
};

MainWindow::MainWindow()
{
    textEdit = new QTextEdit;
    setCentralWidget(textEdit);

    textEdit->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == textEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Ate key press" << keyEvent->key();
            return true;
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}
//! [6]

void move_example(QObject *myObject)
{
    //! [7]
    myObject->moveToThread(QApplication::instance()->thread());
    //! [7]
}

//! [8]
class MyObject : public QObject
{
    Q_OBJECT

public:
    MyObject(QObject *parent = nullptr);

Q_SIGNALS:
    void valueChanged();

protected:
    void timerEvent(QTimerEvent *event) override;
};

MyObject::MyObject(QObject *parent)
    : QObject(parent)
{
    using namespace std::chrono_literals;

    startTimer(50ms);
    startTimer(5s);
    startTimer(10min);
    startTimer(1h);
}

void MyObject::timerEvent(QTimerEvent *event)
{
    qDebug() << "Timer ID:" << event->id();
}
//! [8]

void qpushbutton_examples(QWidget *parentWidget, QObject *monitoredObj, QObject *filterObj)
{
    {
        //! [10]
        QPushButton *button = parentWidget->findChild<QPushButton *>("button1");
        //! [10]
    }

    {
        //! [11]
        QListWidget *list = parentWidget->findChild<QListWidget *>();
        //! [11]
    }

    {
        //! [12]
        QList<QWidget *> widgets = parentWidget->findChildren<QWidget *>("widgetname");
        //! [12]
    }

    {
        //! [13]
        QList<QPushButton *> allPButtons = parentWidget->findChildren<QPushButton *>();
        //! [13]
    }

    {
        //! [14]
        monitoredObj->installEventFilter(filterObj);
        //! [14]
    }
}

//! [15]
class KeyPressEater : public QObject
{
    Q_OBJECT
public:
    KeyPressEater(QObject *parent = nullptr) : QObject(parent) {}
//...

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug("Ate key press %d", keyEvent->key());
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
//! [15]

class MyWindow : public QWidget
{
    Q_OBJECT
public:
    MyWindow();
    void wrapInFunction();
};
void MyWindow::wrapInFunction()
{
    //! [16]
    KeyPressEater *keyPressEater = new KeyPressEater(this);
    QPushButton *pushButton = new QPushButton(this);
    QListView *listView = new QListView(this);

    pushButton->installEventFilter(keyPressEater);
    listView->installEventFilter(keyPressEater);
    //! [16]
}

//! [17]
MyWindow::MyWindow()
{
    QLabel *senderLabel = new QLabel(tr("Name:"));
    QLabel *recipientLabel = new QLabel(tr("Name:", "recipient"));
//! [17]
}

int receivers(const char *signal)
{ return 0; }

void get_the_value(QByteArray *data);

void valueChanged(const QByteArray &data);

void connect_examples()
{
    //! [21]
    if (receivers(SIGNAL(valueChanged(QByteArray))) > 0) {
        QByteArray data;
        get_the_value(&data);       // expensive operation
        emit valueChanged(data);
    }
    //! [21]


    //! [22]
    QLabel *label = new QLabel;
    QScrollBar *scrollBar = new QScrollBar;
    QObject::connect(scrollBar, SIGNAL(valueChanged(int)),
                     label,  SLOT(setNum(int)));
    //! [22]


    //! [23]
    // WRONG
    QObject::connect(scrollBar, SIGNAL(valueChanged(int value)),
                     label, SLOT(setNum(int value)));
    //! [23]
}


//! [24]
class MyWidget : public QWidget
{
    Q_OBJECT

public:
    MyWidget();

signals:
    void buttonClicked();

private:
    QPushButton *myButton;
};

MyWidget::MyWidget()
{
    myButton = new QPushButton(this);
    connect(myButton, SIGNAL(clicked()),
            this, SIGNAL(buttonClicked()));
}
//! [24]

#if 0
//! [25]
QObject::connect: Cannot queue arguments of type 'MyType'
(Make sure 'MyType' is registered using qRegisterMetaType().)
//! [25]
#endif



void snippets_26_to_32(QObject *myObject, QObject *myReceiver, const QMetaMethod &signal)
{
    //! [26]
        QObject::disconnect(myObject, nullptr, nullptr, nullptr);
    //! [26]


    //! [27]
        myObject->disconnect();
    //! [27]


    //! [28]
        QObject::disconnect(myObject, SIGNAL(mySignal()), nullptr, nullptr);
    //! [28]


    //! [29]
        myObject->disconnect(SIGNAL(mySignal()));
    //! [29]


    //! [30]
        QObject::disconnect(myObject, nullptr, myReceiver, nullptr);
    //! [30]


    //! [31]
        myObject->disconnect(myReceiver);
    //! [31]


    //! [32]
        if (signal == QMetaMethod::fromSignal(&MyObject::valueChanged)) {
            // signal is valueChanged
        }
    //! [32]
}

#if 0
//! [33]
void on_<object name>_<signal name>(<signal parameters>);
//! [33]
#endif

//! [34]
void on_button1_clicked();
//! [34]

namespace MyClassExample1
{
    //! [35]
    class MyClass : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("Author", "Pierre Gendron")
        Q_CLASSINFO("URL", "http://www.my-organization.qc.ca")

    public:
        //...
    };
    //! [35]

    //! [37]
    Q_PROPERTY(QString title READ title WRITE setTitle USER true)
    //! [37]
}

namespace MyClassExample2
{
    //! [38]
    class MyClass : public QObject
    {
        Q_OBJECT

    public:
        MyClass(QObject *parent = nullptr);
        ~MyClass();

        enum Priority { High, Low, VeryHigh, VeryLow };
        Q_ENUM(Priority)
        void setPriority(Priority priority);
        Priority priority() const;
    };
    //! [38]
}

void snippets_41_44(QWidget *parentWidget)
{
    //! [41]
    QPushButton *button = parentWidget->findChild<QPushButton *>("button1", Qt::FindDirectChildrenOnly);
    //! [41]


    //! [42]
    QListWidget *list = parentWidget->findChild<QListWidget *>(Qt::FindDirectChildrenOnly);
    //! [42]

    {
        QWidget parentWidget;
        //! [43]
        QList<QPushButton *> childButtons = parentWidget.findChildren<QPushButton *>(Qt::FindDirectChildrenOnly);
        //! [43]
    }
    //! [44]
    QLabel *label = new QLabel;
    QLineEdit *lineEdit = new QLineEdit;
    QObject::connect(lineEdit, &QLineEdit::textChanged,
                    label,  &QLabel::setText);
    //! [44]
}

//! [45]
void someFunction();
//...
void someOtherFunction()
{
    QPushButton *button = new QPushButton;
    QObject::connect(button, &QPushButton::clicked, someFunction);
}
//! [45]


#if __has_include(<QTcpSocket>)
#include <QTcpSocket>

namespace TcpSocketExample
{
    class MyObject : public QObject
    {
        Q_OBJECT

    public:
        MyObject(QObject *parent = nullptr);

        void snippets_46_to_48_51( MyObject *myObject, QLineEdit *lineEdit, QLabel *label)
        {
            //! [46]
                QByteArray page = "...";
                QTcpSocket *socket = new QTcpSocket;
                socket->connectToHost("qt-project.org", 80);
                QObject::connect(socket, &QTcpSocket::connected, [=] () {
                        socket->write("GET " + page + "\r\n");
                    });
            //! [46]

            //! [47]
                QObject::disconnect(myObject, &MyObject::mySignal, nullptr, nullptr);
            //! [47]

            //! [48]
                QObject::disconnect(lineEdit, &QLineEdit::textChanged,
                                label,  &QLabel::setText);
            //! [48]

            //! [49]
                static const QMetaMethod valueChangedSignal = QMetaMethod::fromSignal(&MyObject::valueChanged);
                if (QObject::isSignalConnected(valueChangedSignal)) {
                    QByteArray data;
                    data = get_the_value();       // expensive operation
                    emit valueChanged(data);
                }
            //! [49]

            {
                //! [51]
                QByteArray page = "...";
                QTcpSocket *socket = new QTcpSocket;
                socket->connectToHost("qt-project.org", 80);
                QObject::connect(socket, &QTcpSocket::connected, this, [=] () {
                        socket->write("GET " + page + "\r\n");
                    }, Qt::AutoConnection);
                //! [51]
            }
        }

        QByteArray get_the_value();

        //! [50]
        //...
        void someOtherFunction()
        {
            QPushButton *button = new QPushButton;
            QObject::connect(button, &QPushButton::clicked, this, someFunction, Qt::QueuedConnection);
        }
        //! [50]

    Q_SIGNALS:
        void valueChanged(QByteArray &data);
        void mySignal();

    protected:
        void timerEvent(QTimerEvent *event) override;
    };
    //! [50_someFunction]
    void someFunction();
    //! [50_someFunction]
}
#endif // __has_include(<QTcpSocket>)

void doThings() {}
void doThings2() {}

//! [52]
class MyClass : public QWidget
{
    Q_OBJECT

public:
    MyClass(QWidget *parent = nullptr);
    ~MyClass();

    bool event(QEvent* ev) override
    {
        if (ev->type() == QEvent::PolishRequest) {
            // overwrite handling of PolishRequest if any
            doThings();
            return true;
        } else  if (ev->type() == QEvent::Show) {
            // complement handling of Show if any
            doThings2();
            QWidget::event(ev);
            return true;
        }
        // Make sure the rest of events are handled
        return QWidget::event(ev);
    }
};
//! [52]

void blocker_and_invalid_timer_id_examples(QObject *someQObject)
{

    //! [53]
    {
        const QSignalBlocker blocker(someQObject);
        // no signals here
    }
    //! [53]

    //! [54]
    const bool wasBlocked = someQObject->blockSignals(true);
    // no signals here
    someQObject->blockSignals(wasBlocked);
    //! [54]

    using namespace std::chrono_literals;

    {
    //! [invalid-timer-id]
        QObject *obj;
        //...
        const auto id = Qt::TimerId {obj->startTimer(100ms)};
        if (id != Qt::TimerId::Invalid)
        {
            // The timer has been started successfully
        }
    //! [invalid-timer-id]
    }
}
