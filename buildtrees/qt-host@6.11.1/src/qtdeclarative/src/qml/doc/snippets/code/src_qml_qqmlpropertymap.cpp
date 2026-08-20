// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
class MyQmlPropertyMap : public QQmlPropertyMap
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MyQmlPropertyMap)
public:
    explicit MyQmlPropertyMap(QObject *parent = nullptr)
        : QQmlPropertyMap(this, parent)
    {
        insert("name", "John Smith");
        insert("phone", "555-5555");
        insert("email", "john.smith@example.com");
    }

public slots:
    void updateEmail(const QString &newEmail)
    {
        insert("email", newEmail);
    }
};
//! [0]

void wrapper1() {
//! [1]
    QQuickView view;
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();
//! [1]
}
