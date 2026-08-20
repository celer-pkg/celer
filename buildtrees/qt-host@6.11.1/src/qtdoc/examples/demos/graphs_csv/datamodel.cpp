// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datamodel.h"
#include <QFileInfo>
#include <QQmlFile>
#include <QQmlContext>
#include <charconv>
#include <csv.hpp>
#include <qlogging.h>
#include <qqml.h>
#include <qtpreprocessorsupport.h>
#include <sstream>
#include <string_view>

DataModel::DataModel(QObject *parent) : QAbstractTableModel(parent) { }

DataModel::~DataModel() { }

int DataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_csvData.count() ? m_csvData.at(0).count() : 0;
}


QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (index.row() < rowCount() && index.column() < columnCount())
            return m_csvData.at(index.row()).at(index.column());
    }
    return QVariant();
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section < columnCount())
                return m_csvData.at(0).at(section);
        }
        else if (orientation == Qt::Vertical) {
            if (section < rowCount() && columnCount() > 0)
                return m_csvData.at(section).at(0);
        }
    }
    return QVariant();
}

QHash<int, QByteArray> DataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[CustomRoles::Background] = "background";
    return roles;
}

static int tryConvertToInt(std::string_view team, std::string_view fieldName, std::string field)
{
    int value = -1;
    auto [ptr, ec] = std::from_chars(field.data(), field.data() + field.size(), value);
    Q_UNUSED(ec);
    if (value == -1)
        qWarning("%s: error in %s field", team.data(), fieldName.data());

    return value;
};

void DataModel::readCsv(const QUrl &csvFile)
{
    QAbstractItemModel::beginResetModel();

    const auto context = qmlContext(this);
    const auto resolvedUrl = context ? context->resolvedUrl(csvFile) : csvFile;

    QFile file(QQmlFile::urlToLocalFileOrQrc(resolvedUrl));

    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning("Could not open %s for reading", qUtf8Printable(csvFile.toString()));
        return;
    }

    m_csvData.clear();
    std::stringstream ss(file.readAll().toStdString());

    csv::CSVReader reader(ss);
    auto headers = reader.get_col_names();
    QList<QVariant> headersList;
    headersList.resize(headers.size());
    auto hIt = headersList.begin();
    auto it = headers.begin();
    while (it != headers.end()) {
        *hIt = QString::fromStdString(*it);
        ++hIt;
        ++it;
    }

    if (headersList.count() > 0)
        m_csvData.push_back(headersList);

    for (const csv::CSVRow &csvRow : reader) {
        csv::CSVField teamField = csvRow[headers.at(0)];
        csv::CSVField goldField = csvRow[headers.at(1)];
        csv::CSVField silverField = csvRow[headers.at(2)];
        csv::CSVField bronzeField = csvRow[headers.at(3)];
        csv::CSVField totalField = csvRow[headers.at(4)];

        QList<QVariant> row;
        row.resize(5);
        auto team = teamField.get();
        row[0] = QString::fromStdString(teamField.get());
        row[1] = tryConvertToInt(team, headers.at(1), goldField.get());
        row[2] = tryConvertToInt(team, headers.at(2), silverField.get());
        row[3] = tryConvertToInt(team, headers.at(3), bronzeField.get());
        row[4] = tryConvertToInt(team, headers.at(4), totalField.get());
        m_csvData.push_back(row);
    }

    QAbstractItemModel::endResetModel();
}
