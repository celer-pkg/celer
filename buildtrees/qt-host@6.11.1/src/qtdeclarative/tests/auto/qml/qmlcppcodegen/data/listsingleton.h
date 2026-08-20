#ifndef LISTSINGLETON_H
#define LISTSINGLETON_H

#include <QObject>
#include <QtQml/qqml.h>
#include <QtQml/qqmlregistration.h>

class ListSingleton : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    ListSingleton(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QStringList get() const
    {
        return { QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three") };
    }
};

#endif // LISTSINGLETON_H
