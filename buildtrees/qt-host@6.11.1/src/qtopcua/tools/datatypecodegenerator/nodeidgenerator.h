// Copyright (C) 2024 basysKom GmbH, opensource@basyskom.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef NODEIDGENERATOR_H
#define NODEIDGENERATOR_H

#include <QHash>
#include <QString>

class NodeIdGenerator
{
public:
    NodeIdGenerator() = default;

    bool parseNodeIds(const QString &name, const QString &path);

    bool hasNodeIds() const;

    bool generateNodeIdsHeader(const QString &prefix, const QString &path, const QString &header);

private:
    class NodeId {
    public:
        QString name;
        QString value;
        QString type;
    };

    QHash<QString, QList<NodeId>> m_nodeIds;
};

#endif // NODEIDGENERATOR_H
