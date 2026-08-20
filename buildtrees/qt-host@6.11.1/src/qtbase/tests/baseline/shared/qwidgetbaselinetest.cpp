// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwidgetbaselinetest.h"

#include <qbaselinetest.h>
#include <QApplication>
#include <QStyle>
#include <QStyleHints>
#include <QScreen>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOption>
#include <QJsonObject>
#include <QJsonArray>

#include <QtCore/private/qabstractanimation_p.h>
#include <QtWidgets/private/qapplication_p.h>
#include <QtWidgets/private/qstyle_p.h>

#if defined(Q_OS_APPLE)
#include <QtCore/private/qcore_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

class DebugStyle : public QProxyStyle
{
public:
    DebugStyle(QStyle *style, QWidgetBaselineTest* baselineTest)
        : QProxyStyle(style), baselineTest(baselineTest)
    {
        setParent(baselineTest);
        QStylePrivate::get(this)->name = style->name();
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override
    {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        drawDebugRect("QStyle::drawPrimitive", Qt::magenta, element, option, widget, painter);
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override
    {
        QProxyStyle::drawControl(element, option, painter, widget);
        drawDebugRect("QStyle::drawControl", Qt::magenta, element, option, widget, painter);
    }

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override
    {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        drawDebugRect("QStyle::drawComplexControl", Qt::magenta, control, option, widget, painter);

        // Report all matching sub-controls, independently of whether the above call queries them
        static QMetaEnum complexControlEnum = QMetaEnum::fromType<ComplexControl>();
        static QMetaEnum subControlEnum = QMetaEnum::fromType<SubControl>();
        const auto prefix = QByteArray(complexControlEnum.valueToKey(control)).replace("CC_", "SC_");
        for (int i = 0; i < subControlEnum.keyCount(); ++i) {
            const QByteArray key = subControlEnum.key(i);
            if (key.startsWith(prefix)) {
                auto rect = subControlRect(control, option, SubControl(subControlEnum.value(i)), widget);
                baselineTest->reportDebugRect("QStyle::subControlRect", QColorConstants::Svg::orange,
                    key, rect.translated(option->rect.topLeft()), widget, painter);
            }
        }
    }

private:
    template <typename T>
    void drawDebugRect(const QString &type,  QColor color, T element, const QStyleOption *option, const QWidget *widget, QPainter *painter = nullptr) const
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        auto *elementName = metaEnum.valueToKey(element);

        baselineTest->reportDebugRect(type, color,
            QString::fromLatin1(elementName), option->rect,
            widget, painter);

        if (widget) {
            auto *className = widget->metaObject()->className();
            baselineTest->reportDebugRect("QWidget::rect", Qt::green,
                QString::fromLatin1(className), widget->rect(),
                widget, painter);

            baselineTest->reportDebugRect("QWidget::contentsRect", Qt::green,
                QString::fromLatin1(className), widget->contentsRect(),
                widget, painter);
        }

        if (painter) {
            baselineTest->reportDebugRect("QPainter::clipRegion", Qt::red,
                QString::fromLatin1(elementName), painter->clipRegion().boundingRect(),
                widget, painter);
        }
    }

    QWidgetBaselineTest *baselineTest = nullptr;
};

QWidgetBaselineTest::QWidgetBaselineTest()
{
    // Fail by throwing, since we QVERIFY deep in the helper functions
    QTest::setThrowOnFail(true);

    qApp->setStyle(new DebugStyle(qApp->style(), this));

    QBaselineTest::setProject("Widgets");

    // Set key platform properties that are relevant for the appearance of widgets
    const QString platformName = QGuiApplication::platformName();
    QBaselineTest::addClientProperty("PlatformName", platformName);
    QBaselineTest::addClientProperty("OSVersion", QSysInfo::productVersion());

    // Encode a number of parameters that impact the UI
    QPalette palette;
    QFont font;
    const QString styleName =
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QApplication::style()->metaObject()->className();
#else
            QApplication::style()->name();
#endif
    // turn off animations and make the cursor flash time really long to avoid blinking
    QApplication::style()->setProperty("_qt_animation_time", QTime());
    QApplication::style()->setProperty("_q_no_animation", true);
    QUnifiedTimer::instance()->setSpeedModifier(100000);
    QGuiApplication::styleHints()->setCursorFlashTime(50000);

    QByteArray appearanceBytes;
    {
        QDataStream appearanceStream(&appearanceBytes, QIODevice::WriteOnly);
        appearanceStream << palette << font;
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const quint16 appearanceId = qChecksum(appearanceBytes, appearanceBytes.size());
#else
    const quint16 appearanceId = qChecksum(appearanceBytes);
#endif

    // Assume that text that's darker than the background means we run in light mode
    // This results in a more meaningful appearance ID between different runs than
    // just the checksum of the various attributes.
    const QColor windowColor = palette.window().color();
    const QColor textColor = palette.text().color();
    const QString appearanceIdString = (windowColor.value() > textColor.value()
                                        ? QString("light-%2") : QString("dark-%2"))
                                       .arg(appearanceId, 0, 16);
    QBaselineTest::addClientProperty("AppearanceID", appearanceIdString);

#if defined(Q_OS_APPLE)
    QBaselineTest::addClientProperty("LiquidGlass",
        qt_apple_runningWithLiquidGlass() ? "enabled" : "disabled");
#endif

    QBaselineTest::addClientProperty("DevicePixelRatio",
        QString::number(QGuiApplication::primaryScreen()->devicePixelRatio()));

    QBaselineTest::addClientProperty("Style", styleName);

    QBaselineTest::setProjectImageKeys({
        "GitBranch",
        "OSName",
        "OSVersion",
        "PlatformName",
        "Style",
        "AppearanceID"
    });

    // let users know where they can find the results
    qDebug() << "PlatformName computed to be:" << platformName;
    qDebug() << "Appearance ID computed as:" << appearanceIdString;
}

void QWidgetBaselineTest::initTestCase()
 {
    // Check and setup the environment. Failure to do so skips the test.
    QByteArray msg;
    if (!QBaselineTest::connectToBaselineServer(&msg))
        QSKIP(msg);
}

void QWidgetBaselineTest::init()
{
    QVERIFY(!window);
    background = new QWidget(nullptr, Qt::FramelessWindowHint);
    QPalette pal;

    QImage checkerboard(QSize(20, 20), QImage::Format_Grayscale8);
    checkerboard.fill(Qt::white);
    QPainter painter(&checkerboard);
    painter.fillRect(0, 0, 10, 10, Qt::lightGray);
    painter.fillRect(10, 10, 10, 10, Qt::lightGray);
    painter.end();

    pal.setBrush(QPalette::Window, checkerboard);
    background->setPalette(pal);

    window = new QWidget(background, Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    window->setWindowTitle(QTest::currentDataTag());
    window->setFocusPolicy(Qt::StrongFocus);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    background->setScreen(QGuiApplication::primaryScreen());
    window->setScreen(QGuiApplication::primaryScreen());
#endif
    background->move(QGuiApplication::primaryScreen()->availableGeometry().topLeft());
    window->move(QGuiApplication::primaryScreen()->availableGeometry().topLeft());

    debugRects = QJsonObject{};

    doInit();
}

void QWidgetBaselineTest::cleanup()
{
    doCleanup();

    delete background;
    background = nullptr;
    window = nullptr;
}

void QWidgetBaselineTest::cleanupTestCase()
{
    QBaselineTest::finalizeAndDisconnect();
}

void QWidgetBaselineTest::makeVisible()
{
    Q_ASSERT(window);

    // Always open window on primary screen
    QScreen *preferredScreen = QGuiApplication::primaryScreen();
    const QRect preferredScreenRect = preferredScreen->availableGeometry();

    background->setScreen(preferredScreen);
    background->move(preferredScreenRect.topLeft());
    background->showMaximized();
    QVERIFY(QTest::qWaitForWindowExposed(background));

    window->setScreen(preferredScreen);
    window->move(preferredScreenRect.topLeft());
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QApplicationPrivate::setActiveWindow(window);
    QVERIFY(QTest::qWaitForWindowActive(window));
    // explicitly set focus on the window so that the test widget doesn't have it
    window->setFocus(Qt::OtherFocusReason);
    QTRY_COMPARE(window->focusWidget(), window);
}

/*
    Grabs the test window and returns the resulting QImage, without
    compensating for DPR differences.
*/
QImage QWidgetBaselineTest::takeSnapshot()
{
    // Process events for whatever state changes was initiated
    // prior to the snapshot.
    QCoreApplication::processEvents();

    // Render to QImage instead of going via QWidget::grab(),
    // as the latter will typically use an RGB32 image, and
    // we want to detect issues in the alpha-channel too.
    const auto dpr = window->devicePixelRatio();
    const auto size = window->size();
    QImage image(size * dpr, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    // The widget might claim to be be opaque, but we want to detect if it lies
    image.fill(Qt::transparent);
    window->render(&image, {}, QRect({}, size),
        QWidget::DrawWindowBackground
      | QWidget::DrawChildren
      | QWidget::IgnoreMask
    );

    if (!debugRects.isEmpty()) {
        QJsonDocument doc(debugRects);
        image.setText("DebugRects", doc.toJson(QJsonDocument::Compact));
    }

    return image;

}

/*
    Grabs the test window screen and returns the resulting QImage, without
    compensating for DPR differences.
    This can be used for popup windows.
*/
QImage QWidgetBaselineTest::takeScreenSnapshot(const QRect& windowRect)
{
    // make sure all effects are done - wait longer here because entire
    // windows might be fading in and out.
    QTest::qWait(750);
    return window->screen()->grabWindow(0, windowRect.x(), windowRect.y(),
                                        windowRect.width(), windowRect.height()).toImage();
}

/*!
    Sets standard widget properties on the test window and its children,
    and uploads snapshots. The widgets are returned in the same state
    that they had before.

    Call this helper after setting up the test window.
*/
void QWidgetBaselineTest::takeStandardSnapshots()
{
    makeVisible();

    QWidget *oldFocusWidget = testWindow()->focusWidget();
    QCOMPARE(oldFocusWidget, testWindow());
    QBASELINE_CHECK_DEFERRED(takeSnapshot(), "default");

    // try hard to set focus
    QWidget *testWidget = window->nextInFocusChain();
    if (!testWidget)
        testWidget = window->findChild<QWidget*>();
    QVERIFY(testWidget);
    // use TabFocusReason, some widgets handle that specifically to e.g. select
    testWidget->setFocus(Qt::TabFocusReason);

    if (testWindow()->focusWidget() != oldFocusWidget) {
        QBASELINE_CHECK_DEFERRED(takeSnapshot(), "focused");
        // set focus back
        oldFocusWidget->setFocus(Qt::OtherFocusReason);
    } else {
        qWarning() << "Couldn't set focus on tested widget" << testWidget;
    }

    // this disables all children
    window->setEnabled(false);
    QBASELINE_CHECK_DEFERRED(takeSnapshot(), "disabled");
    window->setEnabled(true);

    // show and activate another window so that our test window becomes inactive
    QWidget otherWindow;
    otherWindow.move(window->geometry().bottomRight() + QPoint(10, 10));
    otherWindow.resize(50, 50);
    otherWindow.setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    otherWindow.show();
    otherWindow.windowHandle()->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&otherWindow));
    QBASELINE_CHECK_DEFERRED(takeSnapshot(), "inactive");

    window->windowHandle()->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));
    if (window->focusWidget())
        window->focusWidget()->clearFocus();
}

void QWidgetBaselineTest::reportDebugRect(const QString &type, const QColor &color,
        const QString &label, QRect widgetRect, const QWidget *widget, QPainter *painter)
{
    const qreal dpr = widget ? widget->devicePixelRatio()
        : painter ? painter->device()->devicePixelRatio()
        : 1.0;

    QRect windowRect = widget ? widgetRect.translated(widget->mapTo(widget->window(), QPoint())) : widgetRect;
    QRect rect(windowRect.topLeft() * dpr, windowRect.size() * dpr);

    auto typeObject = debugRects[type].toObject();

    if (typeObject.isEmpty()) {
        typeObject["color"] = color.name();
        typeObject["rects"] = QJsonArray();
    }

    auto rects = typeObject["rects"].toArray();

    rects.append(QJsonObject{
        { "x", rect.x() },
        { "y", rect.y() },
        { "width", rect.width() },
        { "height", rect.height() },
        { "label", label },
    });

    typeObject["rects"] = rects;
    debugRects[type] = typeObject;
}

QT_END_NAMESPACE
