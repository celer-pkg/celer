// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QEvent>
#include <QtWidgets/qwidget.h>

#include <QtGui/qevent.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>
#include <QtGui/qguiapplication.h>
#include <QQuickView>
#include <QRegularExpression>
#include <QtGlobal>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <memory>
#include <sstream>
#include <vector>
#include <string>

using namespace emscripten;

static std::string toJSArray(const std::vector<std::string> &elements)
{
    std::ostringstream out;
    out << "[";
    bool comma = false;
    for (const auto &element : elements) {
        out << (comma ? "," : "");
        out << element;
        comma = true;
    }
    out << "]";
    return out.str();
}

static std::string toJSString(const QString &qstring)
{
    Q_ASSERT_X(([qstring]() {
                   static QRegularExpression unescapedQuoteRegex(R"re((?:^|[^\\])')re");
                   return qstring.indexOf(unescapedQuoteRegex) == -1;
               })(),
               Q_FUNC_INFO, "Unescaped single quotes found");
    return "'" + qstring.toStdString() + "'";
}

static std::string rectToJSObject(const QRect &rect)
{
    std::ostringstream out;
    out << "{" << "  x: " << std::to_string(rect.x()) << "," << "  y: " << std::to_string(rect.y())
        << "," << "  width: " << std::to_string(rect.width()) << ","
        << "  height: " << std::to_string(rect.height()) << "}";
    return out.str();
}

static std::string screenToJSObject(const QScreen &screen)
{
    std::ostringstream out;
    out << "{" << "  name: " << toJSString(screen.name()) << ","
        << "  geometry: " << rectToJSObject(screen.geometry()) << "}";
    return out.str();
}

static std::string windowToJSObject(const QWindow &window)
{
    std::ostringstream out;
    out << "{" << "  id: " << std::to_string(window.winId()) << ","
        << "  geometry: " << rectToJSObject(window.geometry()) << ","
        << "  frameGeometry: " << rectToJSObject(window.frameGeometry()) << ","
        << "  screen: " << screenToJSObject(*window.screen()) << "," << "  title: '"
        << window.title().toStdString() << "' }";
    return out.str();
}

static void windowInformation()
{
    auto windows = qGuiApp->allWindows();

    std::vector<std::string> windowsAsJsObjects;
    windowsAsJsObjects.reserve(windows.size());
    std::transform(windows.begin(), windows.end(), std::back_inserter(windowsAsJsObjects),
                   [](const QWindow *window) { return windowToJSObject(*window); });

    emscripten::val::global("window").call<void>("windowInformationCallback",
                                                 emscripten::val(toJSArray(windowsAsJsObjects)));
}

static void screenInformation()
{
    auto screens = qGuiApp->screens();

    std::vector<std::string> screensAsJsObjects;
    screensAsJsObjects.reserve(screens.size());
    std::transform(screens.begin(), screens.end(), std::back_inserter(screensAsJsObjects),
                   [](const QScreen *screen) { return screenToJSObject(*screen); });
    emscripten::val::global("window").call<void>("screenInformationCallback",
                                                 emscripten::val(toJSArray(screensAsJsObjects)));
}

QWindow *qmlParentWindow = nullptr;

static void run_QML(const std::string &qmlfile)
{
    QScreen *parentScreen = nullptr;
    {
        auto screens = qGuiApp->screens();
        if (screens.length() > 0)
            parentScreen = screens[0];
        else {
            qWarning() << "No parent screen";
            return;
        }
    }

    qmlParentWindow = new QWindow(parentScreen);
    qmlParentWindow->setFlag(Qt::WindowTitleHint);
    qmlParentWindow->setFlag(Qt::WindowMaximizeButtonHint);
    qmlParentWindow->setTitle("title");
    qmlParentWindow->setGeometry(0, 0, 800, 800);
    qmlParentWindow->setScreen(parentScreen);
    qmlParentWindow->setVisible(true);

    auto view = new QQuickView(qmlParentWindow);
    view->setSource(QUrl(QString::fromUtf8(("qrc:/" + qmlfile).c_str())));
    view->show();
}

static void kill_QML()
{
    qmlParentWindow->deleteLater();
    qmlParentWindow = NULL;
}

#if QT_CONFIG(wasm_jspi)
#  define EMSC_BIND_FUNC(name, afunction) \
      emscripten::function(name, afunction, emscripten::async())
#else
#  define EMSC_BIND_FUNC(name, afunction) \
      emscripten::function(name, afunction)
#endif

EMSCRIPTEN_BINDINGS(qwasmwindow)
{
    EMSC_BIND_FUNC("screenInformation", &screenInformation);
    EMSC_BIND_FUNC("windowInformation", &windowInformation);
    EMSC_BIND_FUNC("run_QML", &run_QML);
    EMSC_BIND_FUNC("kill_QML", &kill_QML);
}

int main(int argc, char **argv)
{
    qputenv("QT_WASM_ENABLE_ACCESSIBILITY", "1");
    QGuiApplication app(argc, argv);

    app.exec();
    return 0;
}
