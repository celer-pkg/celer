// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "stocklistmodel.h"

StockListModel::StockListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_data.append(StockListData("QTCOM.HE", "Qt Group", this));
    m_data.append(StockListData("AAPL", "Apple Inc", this));
    m_data.append(StockListData("MSFT", "Microsoft Corp", this));
    m_data.append(StockListData("AMZN", "Amazon.com Inc", this));
    m_data.append(StockListData("NVDA", "NVIDIA Corp", this));
    m_data.append(StockListData("TSLA", "Tesla Inc", this));
    m_data.append(StockListData("GOOG", "Alphabet Inc", this));
    m_data.append(StockListData("GOOGL", "Alphabet Inc", this));
    m_data.append(StockListData("META", "Meta Platforms Inc", this));
    m_data.append(StockListData("AVGO", "Broadcom Inc", this));
    m_data.append(StockListData("PEP", "PepsiCo Inc", this));
    m_data.append(StockListData("COST", "Costco Wholesale Corp", this));
    m_data.append(StockListData("CSCO", "Cisco Systems Inc", this));
    m_data.append(StockListData("TMUS", "T-Mobile US Inc", this));
    m_data.append(StockListData("CMCSA", "Comcast Corp", this));
    m_data.append(StockListData("TXN", "Texas Instruments Inc", this));
    m_data.append(StockListData("ADBE", "Adobe Inc", this));
    m_data.append(StockListData("NFLX", "Netflix Inc", this));
    m_data.append(StockListData("QCOM", "QUALCOMM Inc", this));
    m_data.append(StockListData("HON", "Honeywell International Inc", this));
    m_data.append(StockListData("AMD", "Advanced Micro Devices Inc", this));
    m_data.append(StockListData("AMGN", "Amgen Inc", this));
    m_data.append(StockListData("SBUX", "Starbucks Corp", this));
    m_data.append(StockListData("INTU", "Intuit Inc", this));
    m_data.append(StockListData("INTC", "Intel Corp", this));
    m_data.append(StockListData("GILD", "Gilead Sciences Inc", this));
    m_data.append(StockListData("AMAT", "Applied Materials Inc", this));
    m_data.append(StockListData("BKNG", "Booking Holdings Inc", this));
    m_data.append(StockListData("ADI", "Analog Devices Inc", this));
    m_data.append(StockListData("ADP", "Automatic Data Processing Inc", this));
    m_data.append(StockListData("MDLZ", "Mondelez International Inc", this));
    m_data.append(StockListData("PYPL", "PayPal Holdings Inc", this));
    m_data.append(StockListData("REGN", "Regeneron Pharmaceuticals Inc", this));
    m_data.append(StockListData("ISRG", "Intuitive Surgical Inc", this));
    m_data.append(StockListData("VRTX", "Vertex Pharmaceuticals Inc", this));
    m_data.append(StockListData("LRCX", "Lam Research Corp", this));
    m_data.append(StockListData("CSX", "CSX Corp", this));
    m_data.append(StockListData("MU", "Micron Technology Inc", this));
    m_data.append(StockListData("MELI", "MercadoLibre Inc", this));
    m_data.append(StockListData("ATVI", "Activision Blizzard Inc", this));
    m_data.append(StockListData("CHTR", "Charter Communications Inc", this));
    m_data.append(StockListData("PANW", "Palo Alto Networks Inc", this));
    m_data.append(StockListData("SNPS", "Synopsys Inc", this));
    m_data.append(StockListData("ASML", "ASML Holding NV", this));
    m_data.append(StockListData("KLAC", "KLA Corp", this));
    m_data.append(StockListData("MAR", "Marriott International Inc/MD", this));
    m_data.append(StockListData("MRNA", "Moderna Inc", this));
    m_data.append(StockListData("MNST", "Monster Beverage Corp", this));
    m_data.append(StockListData("CDNS", "Cadence Design Systems Inc", this));
    m_data.append(StockListData("ORLY", "O'Reilly Automotive Inc", this));
    m_data.append(StockListData("ABNB", "Airbnb Inc", this));
    m_data.append(StockListData("KDP", "Keurig Dr Pepper Inc", this));
    m_data.append(StockListData("KHC", "Kraft Heinz Co/The", this));
    m_data.append(StockListData("FTNT", "Fortinet Inc", this));
    m_data.append(StockListData("NXPI", "NXP Semiconductors NV", this));
    m_data.append(StockListData("AEP", "American Electric Power Co Inc", this));
    m_data.append(StockListData("MCHP", "Microchip Technology Inc", this));
    m_data.append(StockListData("CTAS", "Cintas Corp", this));
    m_data.append(StockListData("ADSK", "Autodesk Inc", this));
    m_data.append(StockListData("DXCM", "Dexcom Inc", this));
    m_data.append(StockListData("PDD", "PDD Holdings Inc ADR", this));
    m_data.append(StockListData("EXC", "Exelon Corp", this));
    m_data.append(StockListData("AZN", "AstraZeneca PLC ADR", this));
    m_data.append(StockListData("PAYX", "Paychex Inc", this));
    m_data.append(StockListData("IDXX", "IDEXX Laboratories Inc", this));
    m_data.append(StockListData("BIIB", "Biogen Inc", this));
    m_data.append(StockListData("MRVL", "Marvell Technology Inc", this));
    m_data.append(StockListData("ROST", "Ross Stores Inc", this));
    m_data.append(StockListData("WBD", "Warner Bros Discovery Inc", this));
    m_data.append(StockListData("LULU", "Lululemon Athletica Inc", this));
    m_data.append(StockListData("PCAR", "PACCAR Inc", this));
    m_data.append(StockListData("ODFL", "Old Dominion Freight Line Inc", this));
    m_data.append(StockListData("WDAY", "Workday Inc", this));
    m_data.append(StockListData("GFS", "GLOBALFOUNDRIES Inc", this));
    m_data.append(StockListData("XEL", "Xcel Energy Inc", this));
    m_data.append(StockListData("CPRT", "Copart Inc", this));
    m_data.append(StockListData("CTSH", "Cognizant Technology Solutions Corp", this));
    m_data.append(StockListData("DLTR", "Dollar Tree Inc", this));
    m_data.append(StockListData("ILMN", "Illumina Inc", this));
    m_data.append(StockListData("WBA", "Walgreens Boots Alliance Inc", this));
    m_data.append(StockListData("BKR", "Baker Hughes Co", this));
    m_data.append(StockListData("EA", "Electronic Arts Inc", this));
    m_data.append(StockListData("FAST", "Fastenal Co", this));
    m_data.append(StockListData("CSGP", "CoStar Group Inc", this));
    m_data.append(StockListData("ENPH", "Enphase Energy Inc", this));
    m_data.append(StockListData("VRSK", "Verisk Analytics Inc", this));
    m_data.append(StockListData("ANSS", "ANSYS Inc", this));
    m_data.append(StockListData("CRWD", "Crowdstrike Holdings Inc", this));
    m_data.append(StockListData("EBAY", "eBay Inc", this));
    m_data.append(StockListData("FANG", "Diamondback Energy Inc", this));
    m_data.append(StockListData("CEG", "Constellation Energy Corp", this));
    m_data.append(StockListData("TEAM", "Atlassian Corp", this));
    m_data.append(StockListData("ALGN", "Align Technology Inc", this));
    m_data.append(StockListData("DDOG", "Datadog Inc", this));
    m_data.append(StockListData("JD", "JD.com Inc ADR", this));
    m_data.append(StockListData("ZS", "Zscaler Inc", this));
    m_data.append(StockListData("ZM", "Zoom Video Communications Inc", this));
    m_data.append(StockListData("RIVN", "Rivian Automotive Inc", this));
    m_data.append(StockListData("SIRI", "Sirius XM Holdings Inc", this));
    m_data.append(StockListData("LCID", "Lucid Group Inc", this));
}

int StockListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.size();
}

int StockListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 8;
}
QVariant StockListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Roles::idRole:
        return m_data[index.row()].stockId;
    case Roles::nameRole:
        return m_data[index.row()].name;
    case Roles::favoriteRole:
        return m_data[index.row()].favorite;
    case Roles::dateRole:
        return m_data[index.row()].model->quoteTime(0);
    case Roles::priceRole:
        return m_data[index.row()].model->price(0);
    case Roles::changeRole:
        return m_data[index.row()].model->change(0);
    case Roles::changePercentageRole:
        return m_data[index.row()].model->changePercentage(0);
    case Roles::filterRole:
        return m_data[index.row()].stockId + " " + m_data.at(index.row()).name;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StockListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Roles::idRole] = "stockId";
    roles[Roles::nameRole] = "name";
    roles[Roles::favoriteRole] = "favorite";
    roles[Roles::dateRole] = "date";
    roles[Roles::priceRole] = "price";
    roles[Roles::changeRole] = "change";
    roles[Roles::changePercentageRole] = "changePercentage";
    roles[Roles::filterRole] = "filter";

    return roles;
}

void StockListModel::updateDetails(const QList<QuoteData> &data)
{
    beginResetModel();
    for (int i = 0; i < m_data.size(); ++i) {
        m_data.at(i).model->appendQuote(data.at(i));
        m_data[i].favorite = m_favorites.contains(m_data.at(i).stockId);
    }
    endResetModel();
}

void StockListModel::resetQuotes()
{
    for (int i = 0; i < m_data.size(); ++i) {
        m_data.at(i).model->resetQuote();
    }
}

void StockListModel::addFavorite(const QString &stockId)
{
    for (int i = 0; i < m_data.size(); i++) {
        StockListData *stock = &m_data[i];
        if (stock->stockId == stockId) {
            beginResetModel();
            stock->favorite = true;
            m_favorites.append(stockId);
            endResetModel();
        }
    }
}
void StockListModel::removeFavorite(const QString &stockId)
{
    for (int i = 0; i < m_data.size(); i++) {
        StockListData *stock = &m_data[i];
        if (stock->stockId == stockId) {
            beginResetModel();
            stock->favorite = false;
            endResetModel();
        }
    }
    for (int i = 0; i < m_favorites.size(); ++i) {
        if (m_favorites.at(i) == stockId)
            m_favorites.removeAt(i);
    }
}

StockModel *StockListModel::stockModel(int index) const
{
    return m_data.at(index).model;
}
