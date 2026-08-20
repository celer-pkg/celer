// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CODECOMPLETIONMODEL_H
#define CODECOMPLETIONMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariant>

class EffectManager;

class CodeCompletionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    struct ModelData {
        QString name;
        int type = 0;
    };

    enum CodeCompletionModelRoles {
        Name = Qt::UserRole + 1,
        Type
    };
    enum WordTypes {
        TypeArgument,
        TypeTag,
        TypeFunction
    };

    explicit CodeCompletionModel(QObject *effectManager);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    void addItem(const QString &text, int type);
    void clearItems();
    CodeCompletionModel::ModelData currentItem();
    int currentIndex() const;
    void setCurrentIndex(int index);

    void nextItem();
    void previousItem();

signals:
    void rowCountChanged();
    void currentIndexChanged();

private:
    friend class CodeHelper;
    QList<ModelData> m_modelList;
    EffectManager *m_effectManager = nullptr;
    int m_currentIndex = 0;
};

#endif // CODECOMPLETIONMODEL_H
