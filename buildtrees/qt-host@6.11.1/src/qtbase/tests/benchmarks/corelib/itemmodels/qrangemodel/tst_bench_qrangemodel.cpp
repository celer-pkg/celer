// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QtCore/qrangemodel.h>
#include <QtCore/qstringlistmodel.h>

class Gadget
{
    Q_GADGET
    Q_PROPERTY(QString display READ display WRITE setDisplay)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QVariant user READ user WRITE setUser)
public:
    Gadget() = default;

    Gadget(const QString &display, const QString &toolTip)
        : m_display(display), m_toolTip(toolTip)
    {
    }

    QString display() const { return m_display; }
    void setDisplay(const QString &display) { m_display = display; }
    QString toolTip() const { return m_toolTip.isEmpty() ? m_display : m_toolTip; }
    void setToolTip(const QString &toolTip) { m_toolTip = toolTip; }
    QVariant user() const { return m_user; }
    void setUser(const QVariant &user) { m_user = user; }

private:
    QString m_display;
    QString m_toolTip;
    QVariant m_user;
};

template <>
struct QRangeModel::RowOptions<Gadget>
{
    static constexpr QRangeModel::RowCategory rowCategory = QRangeModel::RowCategory::MultiRoleItem;
};

struct FastGadget : Gadget
{
    using Gadget::Gadget;
};

template <>
struct QRangeModel::RowOptions<FastGadget> : QRangeModel::RowOptions<Gadget> {};

template <>
struct QRangeModel::ItemAccess<FastGadget>
{
    static QVariant readRole(const FastGadget &item, int role)
    {
        QVariant result;
        switch (role) {
        case Qt::DisplayRole:
            result = item.display();
            break;
        case Qt::ToolTipRole:
            result = item.toolTip();
            break;
        case Qt::UserRole:
            result = item.user();
            break;
        }
        return result;
    }

    static bool writeRole(FastGadget &item, const QVariant &value, int role)
    {
        switch (role) {
        case Qt::DisplayRole:
            item.setDisplay(value.toString());
            break;
        case Qt::ToolTipRole:
            item.setToolTip(value.toString());
            break;
        case Qt::UserRole:
            item.setUser(value);
            break;
        default:
            return false;
        }
        return true;
    }
};

class tst_bench_QRangeModel: public QObject
{
    Q_OBJECT
public:
    tst_bench_QRangeModel(QObject *parent = nullptr);

    enum Type {
        StringListModel,
        VectorStrings,
        ArrayStrings,
    };

private Q_SLOTS:
    void stringList_data();
    void stringList();

    void gadgetReadAccess_data();
    void gadgetReadAccess();

    void gadgetWriteAccess_data() { gadgetReadAccess_data(); }
    void gadgetWriteAccess();
};

tst_bench_QRangeModel::tst_bench_QRangeModel(QObject *parent)
    : QObject(parent)
{
}

void tst_bench_QRangeModel::stringList_data()
{
    QTest::addColumn<Type>("type");
    QTest::addRow("StringListModel") << StringListModel;
    QTest::addRow("VectorStrings") << VectorStrings;
    QTest::addRow("ArrayStrings") << ArrayStrings;
}

void tst_bench_QRangeModel::stringList()
{
    QFETCH(Type, type);

    std::array<QString, 100000> strings;
    for (size_t i = 0; i < std::size(strings); ++i)
        strings[i] = QString::number(i);

    std::unique_ptr<QAbstractItemModel> model;

    switch (type) {
    case StringListModel: {
        model.reset(new QStringListModel(QStringList(std::begin(strings), std::end(strings))));
        break;
    }
    case VectorStrings:
        model.reset(new QRangeModel(std::vector<QString>(std::begin(strings),
                                                         std::end(strings))));
        break;
    case ArrayStrings:
        model.reset(new QRangeModel(strings));
        break;
    }

    QBENCHMARK {
        for (size_t i = 0; i < std::size(strings); ++i) {
            model->data(model->index(i, 0));
        }
    }
}

void tst_bench_QRangeModel::gadgetReadAccess_data()
{
    QTest::addColumn<std::shared_ptr<QRangeModel>>("model");

    QTest::addRow("gadget list") << std::make_shared<QRangeModel>(QList<Gadget> {
            {"display", "tooltip"}
    });

    QTest::addRow("gadget table") << std::make_shared<QRangeModel>(QList<QList<Gadget>> {
        {
            {"display", "tooltip"}
        }
    });

    QTest::addRow("fast gadget list") << std::make_shared<QRangeModel>(QList<FastGadget> {
            {"display", "tooltip"}
    });
}

void tst_bench_QRangeModel::gadgetReadAccess()
{
    QFETCH(std::shared_ptr<QRangeModel>, model);

    const QModelIndex index = model->index(0, 0);
    QBENCHMARK {
        model->data(index, Qt::DisplayRole);
        model->data(index, Qt::UserRole);
    }
}

void tst_bench_QRangeModel::gadgetWriteAccess()
{
    QFETCH(std::shared_ptr<QRangeModel>, model);

    const QModelIndex index = model->index(0, 0);
    const QVariant display = "display";
    const QVariant user = "user";
    QBENCHMARK {
        QVERIFY(model->setData(index, display, Qt::DisplayRole));
        QVERIFY(model->setData(index, user, Qt::UserRole));
    }
}

QTEST_MAIN(tst_bench_QRangeModel)
#include "tst_bench_qrangemodel.moc"
