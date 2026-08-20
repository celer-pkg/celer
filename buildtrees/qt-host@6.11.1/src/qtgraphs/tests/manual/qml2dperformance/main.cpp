// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QDir>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>
#include <qlatin1stringview.h>
#include <qqmlapplicationengine.h>
#include "randomModel.h"
#include "randomGenerator.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  qmlRegisterType<RandomModel>("custom.model", 1,0, "RandomModel");
  qmlRegisterType<RandomGenerator>("custom.model", 1,0, "RandomGenerator");

  QQuickView viewer;

  // The following are needed to make examples run without having to install the
  // module in desktop environments.
#ifdef Q_OS_WIN
  QString extraImportPath(QStringLiteral("%1/../../../%2"));
#else
  QString extraImportPath(QStringLiteral("%1/../../%2"));
#endif
  viewer.engine()->addImportPath(extraImportPath.arg(
      QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));
  QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer,
                   &QWindow::close);

  viewer.setTitle(QStringLiteral("QML 2D performance"));
  viewer.setSource(QUrl("qrc:/qml/qml2dperformance/main.qml"));
  viewer.setResizeMode(QQuickView::SizeRootObjectToView);
  viewer.show();

  return app.exec();
}
