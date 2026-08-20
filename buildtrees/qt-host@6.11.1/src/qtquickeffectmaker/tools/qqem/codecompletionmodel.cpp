// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "codecompletionmodel.h"
#include "effectmanager.h"

CodeCompletionModel::CodeCompletionModel(QObject *effectManager)
    : QAbstractListModel(effectManager)
{
    m_effectManager = static_cast<EffectManager *>(effectManager);
    connect(this, &QAbstractListModel::modelReset, this, &CodeCompletionModel::rowCountChanged);
}

int CodeCompletionModel::rowCount(const QModelIndex &) const
{
    return m_modelList.size();
}

QHash<int, QByteArray> CodeCompletionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Type] = "type";
    return roles;
}

QVariant CodeCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto &data = (m_modelList)[index.row()];

    if (role == Name)
        return QVariant::fromValue(data.name);
    else if (role == Type)
        return QVariant::fromValue(data.type);

    return QVariant();
}

// Note: Doesn't reset the model, do that outside
void CodeCompletionModel::addItem(const QString &text, int type)
{
    ModelData data;
    data.name = text;
    data.type = type;
    m_modelList.append(data);
}

// Note: Doesn't reset the model, do that outside
void CodeCompletionModel::clearItems()
{
    m_modelList.clear();
}

CodeCompletionModel::ModelData CodeCompletionModel::currentItem()
{
    if (m_modelList.size() > m_currentIndex)
        return m_modelList.at(m_currentIndex);

    return CodeCompletionModel::ModelData();
}

int CodeCompletionModel::currentIndex() const
{
    return m_currentIndex;
}

void CodeCompletionModel::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    Q_EMIT currentIndexChanged();
}

void CodeCompletionModel::nextItem()
{
    if (m_modelList.size() > m_currentIndex + 1) {
        m_currentIndex++;
        Q_EMIT currentIndexChanged();
    }
}

void CodeCompletionModel::previousItem()
{
    if (m_currentIndex > 0) {
        m_currentIndex--;
        Q_EMIT currentIndexChanged();
    }
}

