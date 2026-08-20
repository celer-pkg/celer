#pragma once
#include <QObject>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class VariantMapLookupFoo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(QList<QVariantMap> many READ many NOTIFY dataChanged)

public:
    VariantMapLookupFoo(QObject *parent = nullptr) : QObject(parent) { }

    QVariantMap data() const { return m_data; }
    void setData(const QVariantMap &data)
    {
        if (data == m_data)
            return;
        m_data = data;
        emit dataChanged();
    }

    QList<QVariantMap> many() const
    {
        const QVariantMap one = data();
        return QList<QVariantMap>({one, one, one});
    }

signals:
    void dataChanged();

private:
    QVariantMap m_data;
};


