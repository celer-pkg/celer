// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include <QtGraphs/qcategory3daxis.h>
#include <QtGraphs/qitemmodelbardataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qbar3dseries.h>
#include <QtGraphs/qgraphstheme.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTableWidget>
#include <QtGui/QScreen>
#include <QtCore/QRandomGenerator>
#include <QtCore/QTimer>
#include <QtGui/QFont>
#include <QtCore/QDebug>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>

#define USE_STATIC_DATA

class GraphDataGenerator : public QObject
{
public:
    explicit GraphDataGenerator(Q3DBarsWidgetItem *bargraph, QTableWidget *tableWidget);
    ~GraphDataGenerator();

    void setupModel();
    void addRow();
    void changeStyle();
    void changePresetCamera();
    void changeTheme();
    void start();
    void selectFromTable(const QPoint &selection);
    void selectedFromTable(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void fixTableSize();

private:
    Q3DBarsWidgetItem *m_graph;
    QTimer *m_dataTimer;
    int m_columnCount;
    int m_rowCount;
    QTableWidget *m_tableWidget; // not owned
};

GraphDataGenerator::GraphDataGenerator(Q3DBarsWidgetItem *bargraph, QTableWidget *tableWidget)
    : m_graph(bargraph),
      m_dataTimer(0),
      m_columnCount(100),
      m_rowCount(50),
      m_tableWidget(tableWidget)
{
    // Set up bar specifications; make the bars as wide as they are deep,
    // and add a small space between them
    m_graph->setBarThickness(1.0f);
    m_graph->setBarSpacing(QSizeF(0.2, 0.2));

#ifndef USE_STATIC_DATA
    // Set up sample space; make it as deep as it's wide
    m_graph->rowAxis()->setRange(0, m_rowCount);
    m_graph->columnAxis()->setRange(0, m_columnCount);
    m_tableWidget->setColumnCount(m_columnCount);

    // Set selection mode to full
    m_graph->setSelectionMode(QGraphs3DNamespace::SelectionFlag::ItemRowAndColumn);

    // Hide axis labels by explicitly setting one empty string as label list
    m_graph->rowAxis()->setLabels(QStringList(QString()));
    m_graph->columnAxis()->setLabels(QStringList(QString()));

    m_graph->seriesList().at(0)->setItemLabelFormat(QStringLiteral("@valueLabel"));
#else
    // Set selection mode to slice row
    m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::ItemAndRow
                              | QtGraphs3D::SelectionFlag::Slice);
#endif

    // Set theme
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::QtGreen);

    // Set font
    QFont font = QFont("Impact", 20);
    font.setStyleHint(QFont::SansSerif);
    m_graph->activeTheme()->setLabelFont(font);

    // Set preset camera position
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);
}

GraphDataGenerator::~GraphDataGenerator()
{
    if (m_dataTimer) {
        m_dataTimer->stop();
        delete m_dataTimer;
    }
    delete m_graph;
}

void GraphDataGenerator::start()
{
#ifndef USE_STATIC_DATA
    m_dataTimer = new QTimer();
    m_dataTimer->setTimerType(Qt::CoarseTimer);
    QObject::connect(m_dataTimer, &QTimer::timeout, this, &GraphDataGenerator::addRow);
    m_dataTimer->start(0);
    m_tableWidget->setFixedWidth(m_graph->width());
#else
    setupModel();

    // Table needs to be shown before the size of its headers can be accurately obtained,
    // so we postpone it a bit
    m_dataTimer = new QTimer();
    m_dataTimer->setSingleShot(true);
    QObject::connect(m_dataTimer, &QTimer::timeout, this, &GraphDataGenerator::fixTableSize);
    m_dataTimer->start(0);
#endif
}

void GraphDataGenerator::setupModel()
{
    // Set up row and column names
    QStringList days;
    days << "Monday" << "Tuesday" << "Wednesday" << "Thursday" << "Friday" << "Saturday" << "Sunday";
    QStringList weeks;
    weeks << "week 1" << "week 2" << "week 3" << "week 4" << "week 5";

    // Set up data         Mon  Tue  Wed  Thu  Fri  Sat  Sun
    float hours[5][7] = {{2.0f, 1.0f, 3.0f, 0.2f, 1.0f, 5.0f, 10.0f},     // week 1
                         {0.5f, 1.0f, 3.0f, 1.0f, 2.0f, 2.0f, 3.0f},      // week 2
                         {1.0f, 1.0f, 2.0f, 1.0f, 4.0f, 4.0f, 4.0f},      // week 3
                         {0.0f, 1.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.3f},      // week 4
                         {3.0f, 3.0f, 6.0f, 2.0f, 2.0f, 1.0f, 1.0f}};     // week 5

    // Add labels
    m_graph->rowAxis()->setTitle("Week of year");
    m_graph->rowAxis()->setTitleVisible(true);
    m_graph->columnAxis()->setTitle("Day of week");
    m_graph->columnAxis()->setTitleVisible(true);
    m_graph->valueAxis()->setTitle("Hours spent on the Internet");
    m_graph->valueAxis()->setTitleVisible(true);
    m_graph->valueAxis()->setLabelFormat("%.1f h");

    m_tableWidget->setRowCount(5);
    m_tableWidget->setColumnCount(7);
    m_tableWidget->setHorizontalHeaderLabels(days);
    m_tableWidget->setVerticalHeaderLabels(weeks);
    m_tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableWidget->setCurrentCell(-1, -1);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    for (int week = 0; week < weeks.size(); week++) {
        for (int day = 0; day < days.size(); day++) {
            QModelIndex index = m_tableWidget->model()->index(week, day);
            m_tableWidget->model()->setData(index, hours[week][day]);
        }
    }
}

void GraphDataGenerator::addRow()
{
    m_tableWidget->model()->insertRow(0);
    if (m_tableWidget->model()->rowCount() > m_rowCount)
        m_tableWidget->model()->removeRow(m_rowCount);
    for (int i = 0; i < m_columnCount; i++) {
        QModelIndex index = m_tableWidget->model()->index(0, i);
        m_tableWidget->model()->setData(index,
            ((float)i / (float)m_columnCount) / 2.0f +
                                        (float)(QRandomGenerator::global()->bounded(30)) / 100.0f);
    }
    m_tableWidget->resizeColumnsToContents();
}

void GraphDataGenerator::selectFromTable(const QPoint &selection)
{
    m_tableWidget->setFocus();
    m_tableWidget->setCurrentCell(selection.x(), selection.y());
}

void GraphDataGenerator::selectedFromTable(int currentRow, int currentColumn,
                                           int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);
    m_graph->seriesList().at(0)->setSelectedBar(QPoint(currentRow, currentColumn));
}

void GraphDataGenerator::fixTableSize()
{
    int width = m_tableWidget->horizontalHeader()->length();
    width += m_tableWidget->verticalHeader()->width();
    m_tableWidget->setFixedWidth(width + 2);
    int height = m_tableWidget->verticalHeader()->length();
    height += m_tableWidget->horizontalHeader()->height();
    m_tableWidget->setFixedHeight(height + 2);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q3DBarsWidgetItem *graph = new Q3DBarsWidgetItem();

    QWidget widget;
    QQuickWidget qqwidget;
    graph->setWidget(&qqwidget);
    QSize screenSize = graph->widget()->screen()->size();
    graph->widget()->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 2));
    graph->widget()->setMaximumSize(screenSize);
    graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graph->widget()->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout(&widget);
    QTableWidget *tableWidget = new QTableWidget(&widget);
    layout->addWidget(graph->widget(), 1);
    layout->addWidget(tableWidget, 1, Qt::AlignHCenter);

    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableWidget->setAlternatingRowColors(true);
    widget.setWindowTitle(QStringLiteral("Hours spent on the Internet"));

    // Since we are dealing with QTableWidget, the model will already have data sorted properly
    // into rows and columns, so we simply set useModelCategories property to true to utilize this.
    QItemModelBarDataProxy *proxy = new QItemModelBarDataProxy(tableWidget->model());
    proxy->setUseModelCategories(true);
    QBar3DSeries *series = new QBar3DSeries(proxy);
    series->setMesh(QAbstract3DSeries::Mesh::Pyramid);
    graph->addSeries(series);

    GraphDataGenerator generator(graph, tableWidget);
    QObject::connect(series, &QBar3DSeries::selectedBarChanged, &generator,
                     &GraphDataGenerator::selectFromTable);
    QObject::connect(tableWidget, &QTableWidget::currentCellChanged, &generator,
                     &GraphDataGenerator::selectedFromTable);

    widget.show();
    generator.start();
    return app.exec();
}
