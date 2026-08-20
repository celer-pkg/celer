// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PROPERTYMAP_H
#define PROPERTYMAP_H

#include <QtCore/qobject.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlpropertymap.h>

class WithPropertyMap : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQmlPropertyMap *map READ map NOTIFY mapChanged)
public:
    WithPropertyMap(QObject *parent = nullptr)
        : QObject(parent)
        , m_map(QQmlPropertyMap::create(this))
    {
    }

    QQmlPropertyMap *map() const { return m_map; }

    void setProperties(const QVariantHash &properties)
    {
        delete m_map;
        m_map = QQmlPropertyMap::create(this);
        m_map->insert(properties);
        emit mapChanged();
    }

signals:
    void mapChanged();

private:
    QQmlPropertyMap *m_map = nullptr;
};

#endif // PROPERTYMAP_H
