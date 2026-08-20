// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QRasterWindow>
#include <QPainter>
#include <QShortcut>

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

#include <QQmlApplicationEngine>
#include <QQuickView>

QList<std::function<void()>> cleanupFunctions;

#if defined(Q_OS_MACOS)

//! [macos]
#include <AppKit/NSDatePicker.h>
#include <AppKit/NSLayoutConstraint.h>

QWindow *createCalendarWindow()
{
    auto *datePicker = [NSDatePicker new];
    cleanupFunctions.push_back([=]{ [datePicker release]; });

    datePicker.datePickerStyle = NSDatePickerStyleClockAndCalendar;
    datePicker.datePickerElements = NSDatePickerElementFlagYearMonthDay;
    datePicker.drawsBackground = YES;
    datePicker.dateValue = [NSDate now];

    auto *calendarWindow = QWindow::fromWinId(WId(datePicker));
    calendarWindow->setMinimumSize(QSizeF::fromCGSize(datePicker.fittingSize).toSize());

    return calendarWindow;
}
//! [macos]

#elif defined(QT_PLATFORM_UIKIT)

//! [ios]
#include <UIKit/UIDatePicker.h>

QWindow *createCalendarWindow()
{
    auto *datePicker = [UIDatePicker new];
    cleanupFunctions.push_back([=]{ [datePicker release]; });

    datePicker.datePickerMode = UIDatePickerModeDate;
    datePicker.preferredDatePickerStyle = UIDatePickerStyleInline;
    datePicker.backgroundColor = UIColor.systemBackgroundColor;

    auto *calendarWindow = QWindow::fromWinId(WId(datePicker));
    calendarWindow->setMinimumSize(QSizeF::fromCGSize(datePicker.frame.size).toSize());

    return calendarWindow;
}
//! [ios]

#elif defined(Q_OS_WIN)

//! [windows]
#include <windows.h>
#include <commctrl.h>

QWindow *createCalendarWindow()
{
    static bool initializedDateControl = []{
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_DATE_CLASSES;
        return InitCommonControlsEx(&icex);
    }();
    Q_ASSERT(initializedDateControl);

    HWND monthCalendar = CreateWindow(MONTHCAL_CLASSW,
        nullptr, MCS_NOTODAYCIRCLE | MCS_NOTODAY, 0, 0, 0, 0,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    cleanupFunctions.push_back([=]{ DestroyWindow(monthCalendar); });

    auto *calendarWindow = QWindow::fromWinId(WId(monthCalendar));

    RECT minimumSize;
    MonthCal_GetMinReqRect(monthCalendar, &minimumSize);
    const auto dpr = calendarWindow->devicePixelRatio();
    calendarWindow->setMinimumSize(QSize(
        minimumSize.right / dpr,minimumSize.bottom / dpr));

    return calendarWindow;
}
//! [windows]

#elif defined(Q_OS_ANDROID)

//! [android]
Q_DECLARE_JNI_CLASS(CalendarView, "android/widget/CalendarView")
Q_DECLARE_JNI_CLASS(Color, "android/graphics/Color")

QWindow *createCalendarWindow()
{
    using namespace QtJniTypes;
    using namespace QNativeInterface;

    auto *androidApp = qGuiApp->nativeInterface<QAndroidApplication>();
    Q_ASSERT(androidApp);

    auto *calendarView = new CalendarView(androidApp->context());
    cleanupFunctions.push_back([=]{ delete calendarView; });

    // Resolving Android default colors is not trivial, so let's ask Qt
    QColor paletteColor = qGuiApp->palette().color(QPalette::Window);
    int backgroundColor = Color::callStaticMethod<int>("rgb",
        paletteColor.red(), paletteColor.green(), paletteColor.blue());
    calendarView->callMethod<void>("setBackgroundColor", backgroundColor);

    auto *calendarWindow = QWindow::fromWinId(WId(calendarView->object()));
    calendarWindow->setMinimumSize(QSize(200, 220));

    return calendarWindow;
}
//! [android]

#elif QT_CONFIG(xcb) && __has_include(<gtk/gtk.h>)

QT_REQUIRE_CONFIG(glib);

//! [x11]
#include <gtk/gtk.h>
#include <gtk/gtkx.h>

QWindow *createCalendarWindow()
{
    static bool initializedGTK = []{
        qputenv("GDK_BACKEND", "x11");
        return gtk_init_check(nullptr, nullptr);
    }();
    Q_ASSERT(initializedGTK);

    auto *plug = gtk_plug_new(0);
    g_signal_connect(GTK_WIDGET(plug), "delete-event", G_CALLBACK(+[]{
        return true; // Don't destroy on close
    }), nullptr);
    cleanupFunctions.push_back([=]{ gtk_widget_destroy(GTK_WIDGET(plug)); });

    auto *calendar = gtk_calendar_new();
    gtk_container_add(GTK_CONTAINER(plug), GTK_WIDGET(calendar));
    gtk_widget_show_all(plug);

    auto *calendarWindow = QWindow::fromWinId(gtk_plug_get_id(GTK_PLUG(plug)));

    GtkRequisition minimumSize;
    gtk_widget_get_preferred_size(calendar, &minimumSize, NULL);
    calendarWindow->setMinimumSize(QSize(minimumSize.width, minimumSize.height));

    return calendarWindow;
}
//! [x11]

#elif defined(Q_OS_WASM)

//! [webassembly]
#include <emscripten.h>
#include <emscripten/val.h>
using emscripten::val;
using emscripten::EM_VAL;

EM_JS(EM_VAL, createCalendarElement, (), {
    var calendar = document.createElement("calendar-date");
    calendar.innerHTML = "<calendar-month></calendar-month>";
    return Emval.toHandle(calendar);
});

QWindow *createCalendarWindow()
{
    static bool initializedCalendarComponent = []{
        return EM_ASM_INT(
            var script = document.createElement('script');
            script.src = "https://unpkg.com/cally";
            script.type = "module";
            document.head.appendChild(script);
            return true;
        );
    }();
    Q_ASSERT(initializedCalendarComponent);

    val *calendarElement = new val(val::take_ownership(createCalendarElement()));
    cleanupFunctions.push_back([calendarElement]{ delete calendarElement; });

    QWindow *window = QWindow::fromWinId(WId(calendarElement));
    window->setMinimumSize(QSize(250, 300));
    return window;
}
//! [webassembly]

#else

class GrayWindow : public QRasterWindow
{
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        QRectF rect(0, 0, width(), height());
        painter.fillRect(rect, Qt::gray);
        painter.drawText(rect, Qt::AlignCenter,
            "Qt fallback\nforeign window");

    }
};

QWindow *createCalendarWindow()
{
    #warning Using Qt fallback implementation of foreign window

    auto *calendarWindow = new GrayWindow;
    calendarWindow->setMinimumSize(QSize(150, 150));
    return calendarWindow;
}

#endif

constexpr QMargins contentsMargins = {20, 20, 20, 20};

//! [qt-gui-container-window]
class ContainerWindow : public QRasterWindow
{
protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::ChildWindowAdded) {
            auto *childWindow = static_cast<QChildWindowEvent*>(event)->child();
            childWindow->resize(childWindow->minimumSize());
            setMinimumSize(childWindow->size().grownBy(contentsMargins));
            resize(minimumSize());
        }

        return QRasterWindow::event(event);
    }

    void showEvent(QShowEvent *) override
    {
        findChild<QWindow*>()->setVisible(true);
    }

    void resizeEvent(QResizeEvent *) override
    {
        auto *containedWindow = findChild<QWindow*>();
        containedWindow->setPosition(
            (width() / 2)  - containedWindow->width() / 2,
            (height() / 2) - containedWindow->height() / 2
        );
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(0, 0, width(), height(), "#00414A");
    }
};
//! [qt-gui-container-window]

int main(int argc, char* argv[])
{
    QApplication app(argc,argv);
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setOrganizationDomain("qt-project.org");
    QCoreApplication::setApplicationName("Window Embedding");

    QShortcut quitShortcut(QKeySequence::Quit, &app, &app,
        &QCoreApplication::quit, Qt::ApplicationShortcut);

    qAddPostRoutine([]{
        for (auto cleanupFunction : cleanupFunctions)
            cleanupFunction();
    });

    //! [qt-gui]
    ContainerWindow window;
    window.setTitle("Qt Gui");

    auto *calendarWindow = createCalendarWindow();
    calendarWindow->setParent(&window);
    //! [qt-gui]

    //! [qt-widgets]
    QWidget widget;
    widget.setPalette(QColor("#CDB0FF"));
    widget.setWindowTitle("Qt Widgets");
    widget.setLayout(new QVBoxLayout);
    widget.layout()->setContentsMargins(contentsMargins);
    widget.layout()->setAlignment(Qt::AlignCenter);

    auto *calendarWidget = QWidget::createWindowContainer(createCalendarWindow());
    widget.layout()->addWidget(calendarWidget);
    //! [qt-widgets]

    //! [qt-quick]
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{ "calendarWindow", QVariant::fromValue(createCalendarWindow()) }});
    engine.loadFromModule("windowembedding", "Main");
    //! [qt-quick]

    auto &quickWindow = *qobject_cast<QQuickWindow *>(engine.rootObjects().first());

    auto positionWindows = [&]{
        auto screenCenter = app.primaryScreen()->availableGeometry().center();
        widget.adjustSize();
        auto top = screenCenter.y() - widget.height() / 2;
        widget.setGeometry(screenCenter.x() - widget.width() / 2, top, widget.width(), widget.height());
        window.setPosition(QPoint(widget.geometry().left() - window.width() - contentsMargins.left(), top));
        quickWindow.setPosition(QPoint(widget.geometry().right() + contentsMargins.right(), top));
    };
    // Handle mobile devices by dynamically adjusting to the screen geometry
    QObject::connect(app.primaryScreen(), &QScreen::availableGeometryChanged, positionWindows);

    positionWindows();

    widget.showNormal();
    window.showNormal();
    quickWindow.showNormal();

    return app.exec();
}
