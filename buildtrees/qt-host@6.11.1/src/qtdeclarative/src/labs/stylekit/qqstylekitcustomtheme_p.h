// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQSTYLEKITCUSTOMTHEME_P_H
#define QQSTYLEKITCUSTOMTHEME_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQml/QtQml>

QT_BEGIN_NAMESPACE

class QQStyleKitTheme;

class QQStyleKitCustomTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QQmlComponent *theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)
    QML_NAMED_ELEMENT(CustomTheme)

public:
    QQStyleKitCustomTheme(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &newName);

    QQmlComponent *theme() const;
    void setTheme(QQmlComponent *newTheme);

Q_SIGNALS:
    void nameChanged();
    void themeChanged();

private:
    Q_DISABLE_COPY(QQStyleKitCustomTheme)

    QString m_name;
    QQmlComponent *m_theme = nullptr;
};

QT_END_NAMESPACE

#endif // QQSTYLEKITCUSTOMTHEME_P_H
