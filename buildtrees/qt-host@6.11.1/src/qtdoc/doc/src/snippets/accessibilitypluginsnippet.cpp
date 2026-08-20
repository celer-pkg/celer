// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>

class SliderPlugin : public QAccessiblePlugin
{
//! [2]
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.Accessibility.SliderPlugin" FILE "slider.json")
//! [2]

public:
    SliderPlugin() {}

    QStringList keys() const;
    QAccessibleInterface *create(const QString &classname, QObject *object);
};

//! [0]
QStringList SliderPlugin::keys() const
{
    return QStringList() << QLatin1String("QSlider");
}
//! [0]

//! [1]
QAccessibleInterface *SliderPlugin::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = 0;

    if (classname == QLatin1String("QSlider") && object && object->isWidgetType())
        interface = new QAccessibleSlider(static_cast<QWidget *>(object));

    return interface;
}
//! [1]
