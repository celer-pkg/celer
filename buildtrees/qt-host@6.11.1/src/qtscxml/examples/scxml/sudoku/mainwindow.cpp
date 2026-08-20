// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qstringlistmodel.h>
#include <QtCore/qtextstream.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtScxml/qscxmlstatemachine.h>

static int Size = 9;

using namespace Qt::Literals::StringLiterals;

static QVariantList emptyRow()
{
    QVariantList row;
    for (int i = 0; i < Size; i++)
        row.append(QVariant(0));
    return row;
}

static QVariantMap readSudoku(const QString &fileName)
{
    QFile input(fileName);
    input.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream str(&input);
    const QString data = str.readAll();

    QVariantList initRowsVariant;
    const QStringList rows = data.split('\n'_L1);
    for (int i = 0; i < Size; i++) {
        if (i < rows.count()) {
            QVariantList initRowVariant;
            const QStringList row = rows.at(i).split(','_L1);
            for (int j = 0; j < Size; j++) {
                const int val = j < row.count()
                        ? row.at(j).toInt() % (Size + 1) : 0;
                initRowVariant.append(val);
            }
            initRowsVariant.append(QVariant(initRowVariant));
        } else {
            initRowsVariant.append(QVariant(emptyRow()));
        }
    }

    QVariantMap dataVariant;
    dataVariant.insert(u"initState"_s, initRowsVariant);

    return dataVariant;
}

MainWindow::MainWindow(QScxmlStateMachine *machine, QWidget *parent) :
    QWidget(parent),
    m_machine(machine)
{
    const QList<QToolButton *> initVector(Size, nullptr);
    m_buttons = QList<QList<QToolButton *>>(Size, initVector);

    QGridLayout *layout = new QGridLayout(this);

    for (int i = 0; i < Size; i++) {
        for (int j = 0; j < Size; j++) {
            QToolButton *button = new QToolButton(this);
            button->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);
            layout->addWidget(button, i + i / 3, j + j / 3);
            m_buttons[i][j] = button;
            connect(button, &QToolButton::clicked, this, [this, i, j]() {
                QVariantMap data;
                data.insert(u"x"_s, i);
                data.insert(u"y"_s, j);
                m_machine->submitEvent("tap", data);
            });
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            QFrame *hFrame = new QFrame(this);
            hFrame->setFrameShape(QFrame::HLine);
            layout->addWidget(hFrame, 4 * j + 3, 4 * i, 1, 3);

            QFrame *vFrame = new QFrame(this);
            vFrame->setFrameShape(QFrame::VLine);
            layout->addWidget(vFrame, 4 * i, 4 * j + 3, 3, 1);
        }
    }

    m_startButton = new QToolButton(this);
    m_startButton->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);
    m_startButton->setText(tr("Start"));
    layout->addWidget(m_startButton, Size + 3, 0, 1, 3);

    connect(m_startButton, &QAbstractButton::clicked, this, [this]() {
        if (m_machine->isActive("playing"))
            m_machine->submitEvent("stop");
        else
            m_machine->submitEvent("start");
    });

    m_label = new QLabel(tr("unsolved"));
    m_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_label, Size + 3, 4, 1, 3);

    m_undoButton = new QToolButton(this);
    m_undoButton->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);
    m_undoButton->setText(tr("Undo"));
    m_undoButton->setEnabled(false);
    layout->addWidget(m_undoButton, Size + 3, 8, 1, 3);

    connect(m_undoButton, &QAbstractButton::clicked, this, [this]() {
        m_machine->submitEvent("undo");
    });

    m_chooser = new QComboBox(this);
    layout->addWidget(m_chooser, Size + 4, 0, 1, 11);

    QDir dataDir(":/data"_L1);
    QFileInfoList sudokuFiles = dataDir.entryInfoList(QStringList()
                                                      << "*.data");
    for (const QFileInfo &sudokuFile : sudokuFiles) {
        m_chooser->addItem(sudokuFile.completeBaseName(),
                           sudokuFile.absoluteFilePath());
    }

    connect(m_chooser, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QString sudokuFile = m_chooser->itemData(index).toString();
        const QVariantMap initValues = readSudoku(sudokuFile);
        m_machine->submitEvent("setup", initValues);
    });

    const QVariantMap initValues = readSudoku(
                m_chooser->itemData(0).toString());
    m_machine->setInitialValues(initValues);

    m_machine->connectToState("playing", [this] (bool playing) {
        if (playing) {
            m_startButton->setText(tr("Stop"));
            m_undoButton->setEnabled(true);
            m_chooser->setEnabled(false);
        } else {
            m_startButton->setText(tr("Start"));
            m_undoButton->setEnabled(false);
            m_chooser->setEnabled(true);
        }
    });

    m_machine->connectToState("solved", [this](bool solved) {
        if (solved)
            m_label->setText(tr("SOLVED !!!"));
        else
            m_label->setText(tr("unsolved"));
    });

    m_machine->connectToEvent("updateGUI", [this](const QScxmlEvent &event) {
        const QVariant data = event.data();

        const QVariantList currentRows = data.toMap().value(
                    "currentState").toList();
        for (int i = 0; i < currentRows.count(); i++) {
            const QVariantList row = currentRows.at(i).toList();
            for (int j = 0; j < row.count(); j++) {
                const int value = row.at(j).toInt();
                const QString text = value ? QString::number(value) : QString();
                m_buttons[i][j]->setText(text);
            }
        }

        const bool active = m_machine->isActive("playing");

        const QVariantList initRows = data.toMap().value("initState").toList();
        for (int i = 0; i < initRows.count(); i++) {
            const QVariantList row = initRows.at(i).toList();
            for (int j = 0; j < row.count(); j++) {
                const int value = row.at(j).toInt();
                // enable only zeroes from initState
                const bool enabled = !value && active;
                m_buttons[i][j]->setEnabled(enabled);
            }
        }
    });
}
